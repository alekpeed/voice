package com.alekpeed.voiceleadinglab.domain

import org.junit.Assert.*
import org.junit.Test

class TrainingEnginesTest {
    @Test fun generatorCoversAllKeysAndModes() {
        val generator = ExerciseGenerator()
        for (key in 0..11) assertNotNull(generator.generate(8, key, 2, ExerciseMode.GUIDE_TONES))
        ExerciseMode.entries.forEach { assertNotNull(generator.generate(9, 0, 4, it)) }
    }
    @Test fun generatedTargetPasses() {
        val generator = ExerciseGenerator(); val prompt = generator.generate(12, 5, 3)!!
        assertTrue(generator.submit(prompt, prompt.target))
    }
    @Test fun barryFieldsAreExact() {
        val field = BarryHarrisEngine.field(0, false)
        assertEquals(setOf(0,4,7,9), field.sixth)
        assertEquals(setOf(2,5,8,11), field.diminished)
        assertEquals(listOf(1,4,7,10), BarryHarrisEngine.relatedDominants(field))
    }
    @Test fun earPromptsReproduce() {
        assertEquals(EarTrainingEngine.generate(42, EarTask.DIRECTION), EarTrainingEngine.generate(42, EarTask.DIRECTION))
    }
    @Test fun sessionAndProgressionAreDeterministic() {
        val events = listOf(Sonority(listOf(MidiPitch(48),MidiPitch(52))), Sonority(listOf(MidiPitch(48),MidiPitch(53))))
        assertEquals(MotionHabits(1,1,0), SessionAnalyzer.analyze(events))
        assertTrue(ProgressionPractice.evaluate(PracticePlan("VL-17",listOf(PracticeChord("C",setOf(0,4,7))),1,2),
            listOf(Sonority(listOf(MidiPitch(48),MidiPitch(52))))))
    }
    @Test fun tutorNeverInventsFacts() {
        assertEquals("No deterministic analysis is available.", StructuredTutor.offlineExplanation("VL-01.1", emptyList()))
        assertEquals("VL-01.1: C stayed common.", StructuredTutor.offlineExplanation("VL-01.1", listOf(TutorFact("common","C stayed common."))))
    }
    @Test fun instrumentDefinitionsValidate() {
        assertTrue(InstrumentCatalog.presets.all(InstrumentCatalog::validate))
        assertTrue(InstrumentCatalog.presets.any { it.family == InstrumentFamily.RHODES })
        assertTrue(InstrumentCatalog.presets.any { it.family == InstrumentFamily.WURLITZER })
    }
}
