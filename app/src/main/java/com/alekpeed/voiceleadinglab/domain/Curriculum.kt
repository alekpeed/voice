package com.alekpeed.voiceleadinglab.domain

enum class Competency { NOT_STARTED, INTRODUCED, DEVELOPING, RELIABLE, FLUENT }

data class BookRoute(
    val conceptId: String,
    val studyPage: Int,
    val practicePage: Int,
    val unitId: String,
    val exerciseIds: List<String>,
)

data class Concept(
    val id: String,
    val title: String,
    val prerequisiteId: String?,
    val route: BookRoute,
)

data class Evidence(
    val attempts: Int = 0,
    val successes: Int = 0,
    val unassisted: Int = 0,
    val transposedUnassisted: Int = 0,
) {
    val competency: Competency get() = when {
        unassisted >= 4 && transposedUnassisted >= 2 -> Competency.FLUENT
        unassisted >= 3 -> Competency.RELIABLE
        successes >= 2 -> Competency.DEVELOPING
        attempts > 0 -> Competency.INTRODUCED
        else -> Competency.NOT_STARTED
    }
}

object CurriculumCatalog {
    private val chapterTitles = listOf(
        "Horizontal Hearing", "Motion Between Voices", "Tendency and Resolution",
        "Triads as Voice Networks", "Seventh Chords and Guide Tones", "Bass as Counterline",
        "Cycles and Turnarounds", "Inner Lines", "Delayed Resolution", "Chromatic Motion",
        "Jazz Voicings", "Extensions and Alterations", "Harmonizing Melody",
        "Barry Harris Voice Leading", "Modal and Quartal Motion", "Analytical Reduction",
    )
    val concepts: List<Concept> = buildList {
        var previous: String? = null
        chapterTitles.forEachIndexed { chapterIndex, chapterTitle ->
            repeat(4) { conceptIndex ->
                val chapter = chapterIndex + 1
                val id = "VL-${chapter.toString().padStart(2, '0')}.${conceptIndex + 1}"
                val unit = "VL-U${chapter.toString().padStart(2, '0')}"
                add(Concept(id, "$chapterTitle ${conceptIndex + 1}", previous,
                    BookRoute(id, 7 + chapterIndex * 2, 4 + chapterIndex * 2, unit,
                        ('A'..'E').map { "VL-EX-${chapter.toString().padStart(2, '0')}$it" })))
                previous = id
            }
        }
    }
    fun concept(id: String) = concepts.find { it.id == id }
}

class CompetencyTracker {
    private val evidence = mutableMapOf<String, Evidence>()
    fun record(id: String, success: Boolean, hints: Boolean, transposed: Boolean) {
        if (CurriculumCatalog.concept(id) == null) return
        val old = evidence[id] ?: Evidence()
        evidence[id] = old.copy(
            attempts = old.attempts + 1,
            successes = old.successes + if (success) 1 else 0,
            unassisted = old.unassisted + if (success && !hints) 1 else 0,
            transposedUnassisted = old.transposedUnassisted + if (success && !hints && transposed) 1 else 0,
        )
    }
    fun evidence(id: String) = evidence[id] ?: Evidence()
    fun isUnlocked(id: String): Boolean {
        val prerequisite = CurriculumCatalog.concept(id)?.prerequisiteId ?: return true
        return evidence(prerequisite).competency >= Competency.RELIABLE
    }
}
