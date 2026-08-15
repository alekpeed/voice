package com.alekpeed.voiceleadinglab.platform

import android.content.Context
import android.media.midi.MidiDevice
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiManager
import android.media.midi.MidiReceiver
import android.os.Handler
import android.os.Looper
import java.io.Closeable

data class MidiNoteEvent(val note: Int, val velocity: Int, val on: Boolean, val timestampNanos: Long)

class AndroidMidiInput(context: Context) : Closeable {
    private val manager = context.getSystemService(MidiManager::class.java)
    private var device: MidiDevice? = null
    private var outputPort: android.media.midi.MidiOutputPort? = null
    private val listeners = mutableSetOf<(MidiNoteEvent) -> Unit>()

    fun devices(): List<MidiDeviceInfo> = manager.devices.toList()
    fun addListener(listener: (MidiNoteEvent) -> Unit) { listeners += listener }
    fun removeListener(listener: (MidiNoteEvent) -> Unit) { listeners -= listener }

    fun connect(info: MidiDeviceInfo, onResult: (Boolean) -> Unit) {
        closeDevice()
        manager.openDevice(info, { opened ->
            device = opened
            outputPort = opened?.openOutputPort(0)?.also { it.connect(receiver) }
            onResult(outputPort != null)
        }, Handler(Looper.getMainLooper()))
    }

    private val receiver = object : MidiReceiver() {
        override fun onSend(data: ByteArray, offset: Int, count: Int, timestamp: Long) {
            var index = offset
            val end = offset + count
            while (index + 2 < end) {
                val status = data[index].toInt() and 0xF0
                val note = data[index + 1].toInt() and 0x7F
                val velocity = data[index + 2].toInt() and 0x7F
                if (status == 0x90 || status == 0x80) {
                    val event = MidiNoteEvent(note, velocity, status == 0x90 && velocity > 0, timestamp)
                    listeners.toList().forEach { it(event) }
                }
                index += 3
            }
        }
    }

    private fun closeDevice() {
        outputPort?.close(); outputPort = null
        device?.close(); device = null
    }
    override fun close() = closeDevice()
}
