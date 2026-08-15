package com.alekpeed.voiceleadinglab.domain

import org.junit.Assert.*
import org.junit.Test

class CurriculumTest {
    @Test fun catalogPreservesStableCoverage() {
        assertEquals(64, CurriculumCatalog.concepts.size)
        assertEquals("VL-01.1", CurriculumCatalog.concepts.first().id)
        assertEquals("VL-16.4", CurriculumCatalog.concepts.last().id)
        assertEquals(5, CurriculumCatalog.concept("VL-05.2")!!.route.exerciseIds.size)
    }

    @Test fun evidenceThresholdsMatchBooks() {
        val tracker = CompetencyTracker()
        repeat(2) { tracker.record("VL-01.1", true, true, false) }
        assertEquals(Competency.DEVELOPING, tracker.evidence("VL-01.1").competency)
        repeat(2) { tracker.record("VL-01.1", true, false, true) }
        tracker.record("VL-01.1", true, false, false)
        assertEquals(Competency.RELIABLE, tracker.evidence("VL-01.1").competency)
        tracker.record("VL-01.1", true, false, false)
        assertEquals(Competency.FLUENT, tracker.evidence("VL-01.1").competency)
    }

    @Test fun prerequisitesUnlockOnlyAtReliable() {
        val tracker = CompetencyTracker()
        assertFalse(tracker.isUnlocked("VL-01.2"))
        repeat(3) { tracker.record("VL-01.1", true, false, false) }
        assertTrue(tracker.isUnlocked("VL-01.2"))
    }
}
