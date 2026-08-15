package com.alekpeed.voiceleadinglab.domain

import org.junit.Assert.*
import org.junit.Test

class MusicTest {
    @Test fun pitchClassNormalizes() {
        assertEquals(11, MidiPitch(-1).pitchClass)
        assertEquals(0, MidiPitch(60).pitchClass)
    }

    @Test fun canonicalGuideToneResolutionIsNearest() {
        val solution = NearestVoicingSolver().solve(VoicingRequest(
            source = Sonority(listOf(MidiPitch(53), MidiPitch(60))),
            allowedPitchClasses = setOf(5, 11), requiredPitchClasses = setOf(5, 11),
            voiceCount = 2, minimumMidi = 48, maximumMidi = 72,
        ))
        assertNotNull(solution)
        assertEquals(listOf(MidiPitch(53), MidiPitch(59)), solution!!.sonority.sorted)
        assertEquals(1, solution.result.totalDisplacement)
    }

    @Test fun sopranoLockIsHardConstraint() {
        val solution = NearestVoicingSolver().solve(VoicingRequest(
            Sonority(listOf(MidiPitch(48), MidiPitch(52), MidiPitch(55))),
            setOf(5, 9, 0), voiceCount = 3, lockedSoprano = MidiPitch(60),
        ))
        assertEquals(MidiPitch(60), solution!!.sonority.sorted.last())
    }

    @Test fun voiceLeadingFactsAreExact() {
        val result = VoiceLeadingEvaluator.evaluate(
            Sonority(listOf(MidiPitch(53), MidiPitch(60))),
            Sonority(listOf(MidiPitch(52), MidiPitch(59))),
        )
        assertEquals(2, result.totalDisplacement)
        assertEquals(listOf(-1, -1), result.movements.map { it.semitones })
    }
}
