package com.alekpeed.voiceleadinglab.platform

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import java.io.Closeable
import kotlin.concurrent.thread
import kotlin.math.PI
import kotlin.math.pow
import kotlin.math.sin

class AndroidAudioEngine : Closeable {
    private data class Voice(var phase: Double, val increment: Double, val velocity: Float, var gain: Float = 1f, var released: Boolean = false)
    private val sampleRate = 48_000
    private val voices = mutableMapOf<Int, Voice>()
    private val minimum = AudioTrack.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_FLOAT)
    private val track = AudioTrack.Builder()
        .setAudioAttributes(AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_MEDIA).setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build())
        .setAudioFormat(AudioFormat.Builder().setSampleRate(sampleRate).setEncoding(AudioFormat.ENCODING_PCM_FLOAT).setChannelMask(AudioFormat.CHANNEL_OUT_MONO).build())
        .setBufferSizeInBytes(maxOf(minimum, 4096)).setTransferMode(AudioTrack.MODE_STREAM).build()
    @Volatile private var running = true
    private val renderThread = thread(name = "VLL-Audio", priority = Thread.MAX_PRIORITY) { render() }

    init { track.play() }
    fun noteOn(note: Int, velocity: Int) = synchronized(voices) {
        val frequency = 440.0 * 2.0.pow((note - 69) / 12.0)
        voices[note] = Voice(0.0, 2.0 * PI * frequency / sampleRate, velocity.coerceIn(1,127) / 127f)
    }
    fun noteOff(note: Int) = synchronized(voices) { voices[note]?.released = true }
    fun allNotesOff() = synchronized(voices) { voices.values.forEach { it.released = true } }

    private fun render() {
        val buffer = FloatArray(256)
        while (running) {
            buffer.fill(0f)
            synchronized(voices) {
                val iterator = voices.iterator()
                while (iterator.hasNext()) {
                    val (_, voice) = iterator.next()
                    for (index in buffer.indices) {
                        val fundamental = sin(voice.phase)
                        val tine = sin(voice.phase * 2.01) * .22 + sin(voice.phase * 3.99) * .08
                        buffer[index] += ((fundamental + tine) * voice.velocity * voice.gain * .12).toFloat()
                        voice.phase += voice.increment
                        if (voice.released) voice.gain *= .9985f
                    }
                    if (voice.gain < .0005f) iterator.remove()
                }
            }
            track.write(buffer, 0, buffer.size, AudioTrack.WRITE_BLOCKING)
        }
    }
    override fun close() { running = false; renderThread.join(500); track.pause(); track.flush(); track.release() }
}
