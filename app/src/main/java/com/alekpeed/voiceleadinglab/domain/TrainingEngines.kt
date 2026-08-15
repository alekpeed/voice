package com.alekpeed.voiceleadinglab.domain

import kotlin.math.abs

enum class ExerciseMode { NEAREST, FIXED_SOPRANO, FIXED_BASS, GUIDE_TONES }
data class ExercisePrompt(val id: String, val seed: Long, val key: Int, val mode: ExerciseMode,
    val source: Sonority, val target: Sonority, val requiredPitchClasses: Set<Int>)

class ExerciseGenerator(private val solver: NearestVoicingSolver = NearestVoicingSolver()) {
    fun generate(seed: Long, key: Int = ((seed xor (seed ushr 32)) % 12).toInt().let { if (it < 0) it + 12 else it },
                 voices: Int = 2, mode: ExerciseMode = ExerciseMode.NEAREST): ExercisePrompt? {
        val count = if (mode == ExerciseMode.GUIDE_TONES) 2 else voices
        if (key !in 0..11 || count !in 2..4) return null
        val sourcePcs = setOf((key + 2) % 12, (key + 5) % 12, (key + 9) % 12, key)
        val destinationPcs = setOf((key + 7) % 12, (key + 11) % 12, (key + 2) % 12, (key + 5) % 12)
        val source = sourceVoicing(sourcePcs, count)
        val guideTones = setOf((key + 11) % 12, (key + 5) % 12)
        val nearest = solver.solve(VoicingRequest(source, destinationPcs,
            if (count == 4) destinationPcs else guideTones, count)) ?: return null
        val locked = when (mode) {
            ExerciseMode.FIXED_BASS -> solver.solve(VoicingRequest(source, destinationPcs, guideTones, count, lockedBass = nearest.sonority.sorted.first()))
            ExerciseMode.FIXED_SOPRANO -> solver.solve(VoicingRequest(source, destinationPcs, guideTones, count, lockedSoprano = nearest.sonority.sorted.last()))
            else -> nearest
        } ?: return null
        return ExercisePrompt("VL-05.2-${mode.name}", seed, key, mode, source, locked.sonority, if (count == 4) destinationPcs else guideTones)
    }
    fun submit(prompt: ExercisePrompt, response: Sonority): Boolean =
        response.pitches.size == prompt.target.pitches.size &&
            response.pitches.map { it.pitchClass }.containsAll(prompt.requiredPitchClasses) &&
            VoiceLeadingEvaluator.evaluate(prompt.source, response).totalDisplacement ==
            VoiceLeadingEvaluator.evaluate(prompt.source, prompt.target).totalDisplacement

    private fun sourceVoicing(pcs: Set<Int>, voices: Int): Sonority {
        val notes = (48..72).filter { MidiPitch(it).pitchClass in pcs }
        return Sonority(notes.take(voices).map(::MidiPitch))
    }
}

data class BarryField(val key: Int, val minor: Boolean, val sixth: Set<Int>, val diminished: Set<Int>)
object BarryHarrisEngine {
    fun field(key: Int, minor: Boolean): BarryField {
        fun pc(value: Int) = ((value % 12) + 12) % 12
        return BarryField(pc(key), minor,
            setOf(pc(key), pc(key + if (minor) 3 else 4), pc(key + 7), pc(key + 9)),
            setOf(pc(key + 2), pc(key + 5), pc(key + 8), pc(key + 11)))
    }
    fun relatedDominants(field: BarryField) = field.diminished.map { ((it - 4) % 12 + 12) % 12 }.sorted()
    fun borrow(voicing: Sonority, field: BarryField, count: Int): Sonority {
        val result = voicing.sorted.toMutableList()
        repeat(count.coerceIn(0, result.size)) { index ->
            val original = result[index]
            result[index] = (-2..2).map { MidiPitch(original.note + it) }
                .filter { it.pitchClass in field.diminished }.minBy { abs(it.note - original.note) }
        }
        return Sonority(result.sorted())
    }
}

enum class EarTask { DIRECTION, COMMON_TONES, ISOLATED_VOICE, RECONSTRUCTION }
data class EarPrompt(val seed: Long, val task: EarTask, val progression: List<Sonority>, val answer: Int)
object EarTrainingEngine {
    fun generate(seed: Long, task: EarTask): EarPrompt {
        val key = ((seed xor (seed ushr 32)) % 12).toInt().let { if (it < 0) it + 12 else it }
        val progression = listOf(Sonority(listOf(MidiPitch(48+key),MidiPitch(52+key),MidiPitch(55+key))),
            Sonority(listOf(MidiPitch(48+key),MidiPitch(51+key),MidiPitch(55+key)),1_000_000))
        return EarPrompt(seed, task, progression, when(task){ EarTask.DIRECTION -> -1; EarTask.COMMON_TONES -> 2; EarTask.ISOLATED_VOICE -> 51+key; EarTask.RECONSTRUCTION -> 1 })
    }
}

data class MotionHabits(val commonTones: Int, val steps: Int, val leaps: Int)
object SessionAnalyzer {
    fun analyze(events: List<Sonority>): MotionHabits {
        var common = 0; var steps = 0; var leaps = 0
        events.zipWithNext().forEach { (a,b) -> VoiceLeadingEvaluator.evaluate(a,b).movements.forEach {
            when(it.distance){ 0 -> common++; 1,2 -> steps++; else -> leaps++ }
        }}
        return MotionHabits(common, steps, leaps)
    }
}

data class PracticeChord(val symbol: String, val pitchClasses: Set<Int>)
data class PracticePlan(val id: String, val progression: List<PracticeChord>, val choruses: Int, val voices: Int)
object ProgressionPractice {
    fun evaluate(plan: PracticePlan, response: List<Sonority>): Boolean {
        if (plan.id.isBlank() || plan.progression.isEmpty() || plan.choruses < 1 || plan.voices !in 2..4 ||
            response.size != plan.progression.size * plan.choruses) return false
        return response.withIndex().all { (index, sonority) -> sonority.pitches.size == plan.voices &&
            sonority.pitches.all { it.pitchClass in plan.progression[index % plan.progression.size].pitchClasses } }
    }
}

data class TutorFact(val code: String, val fact: String)
object StructuredTutor {
    fun offlineExplanation(conceptId: String, facts: List<TutorFact>) =
        if (conceptId.isBlank() || facts.isEmpty()) "No deterministic analysis is available."
        else "$conceptId: ${facts.joinToString(" ") { it.fact }}"
}
