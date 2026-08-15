package com.alekpeed.voiceleadinglab.domain

import kotlin.math.abs

@JvmInline value class MidiPitch(val note: Int) : Comparable<MidiPitch> {
    val pitchClass: Int get() = ((note % 12) + 12) % 12
    override fun compareTo(other: MidiPitch) = note.compareTo(other.note)
}

data class Sonority(val pitches: List<MidiPitch>, val timestampMicros: Long = 0) {
    init { require(pitches.distinct().size == pitches.size) }
    val sorted: List<MidiPitch> = pitches.sorted()
}

data class VoiceMovement(val voice: Int, val from: MidiPitch, val to: MidiPitch) {
    val semitones = to.note - from.note
    val distance = abs(semitones)
}

data class VoiceLeadingResult(
    val movements: List<VoiceMovement>,
    val totalDisplacement: Int,
    val maximumMovement: Int,
    val crossing: Boolean,
)

object VoiceLeadingEvaluator {
    fun evaluate(source: Sonority, destination: Sonority): VoiceLeadingResult {
        require(source.sorted.size == destination.sorted.size)
        val moves = source.sorted.zip(destination.sorted).mapIndexed { index, (from, to) ->
            VoiceMovement(index + 1, from, to)
        }
        return VoiceLeadingResult(
            moves,
            moves.sumOf { it.distance },
            moves.maxOfOrNull { it.distance } ?: 0,
            destination.pitches.zipWithNext().any { (a, b) -> a.note >= b.note },
        )
    }
}

data class VoicingRequest(
    val source: Sonority,
    val allowedPitchClasses: Set<Int>,
    val requiredPitchClasses: Set<Int> = emptySet(),
    val voiceCount: Int = source.pitches.size,
    val minimumMidi: Int = 36,
    val maximumMidi: Int = 84,
    val maximumLeap: Int = 12,
    val maximumAdjacentSpacing: Int = 19,
    val lockedBass: MidiPitch? = null,
    val lockedSoprano: MidiPitch? = null,
)

data class VoicingSolution(val sonority: Sonority, val result: VoiceLeadingResult)

class NearestVoicingSolver {
    fun solve(request: VoicingRequest): VoicingSolution? {
        if (request.voiceCount !in 2..4 || request.source.pitches.size != request.voiceCount) return null
        val candidates = (request.minimumMidi..request.maximumMidi)
            .filter { MidiPitch(it).pitchClass in request.allowedPitchClasses }
        var best: VoicingSolution? = null
        fun search(start: Int, chosen: MutableList<MidiPitch>) {
            if (chosen.size == request.voiceCount) {
                if (!chosen.map { it.pitchClass }.containsAll(request.requiredPitchClasses)) return
                if (request.lockedBass != null && chosen.first() != request.lockedBass) return
                if (request.lockedSoprano != null && chosen.last() != request.lockedSoprano) return
                if (chosen.zipWithNext().any { (a, b) -> b.note - a.note > request.maximumAdjacentSpacing }) return
                val result = VoiceLeadingEvaluator.evaluate(request.source, Sonority(chosen.toList()))
                if (result.maximumMovement > request.maximumLeap) return
                val solution = VoicingSolution(Sonority(chosen.toList()), result)
                val current = best
                if (current == null || compare(solution, current) < 0) best = solution
                return
            }
            for (index in start until candidates.size) {
                chosen += MidiPitch(candidates[index]); search(index + 1, chosen); chosen.removeAt(chosen.lastIndex)
            }
        }
        search(0, mutableListOf())
        return best
    }

    private fun compare(a: VoicingSolution, b: VoicingSolution): Int =
        compareValuesBy(a, b,
            { it.result.totalDisplacement },
            { it.result.maximumMovement },
            { it.sonority.sorted.last().note - it.sonority.sorted.first().note },
            { it.sonority.sorted.joinToString(",") { pitch -> pitch.note.toString().padStart(3, '0') } },
        )
}
