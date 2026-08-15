package com.alekpeed.voiceleadinglab.platform

import android.content.Context
import android.media.midi.MidiDeviceInfo
import java.io.Closeable

class PerformanceController(context: Context) : Closeable {
    private val midi = AndroidMidiInput(context)
    private val audio = AndroidAudioEngine()
    init { midi.addListener { event -> if (event.on) audio.noteOn(event.note, event.velocity) else audio.noteOff(event.note) } }
    fun devices(): List<MidiDeviceInfo> = midi.devices()
    fun connect(info: MidiDeviceInfo, result: (Boolean) -> Unit) = midi.connect(info, result)
    override fun close() { midi.close(); audio.close() }
}
