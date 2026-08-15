package com.alekpeed.voiceleadinglab.platform

import android.content.Context
import androidx.datastore.preferences.core.*
import androidx.datastore.preferences.preferencesDataStore
import com.alekpeed.voiceleadinglab.domain.Competency
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

private val Context.progressDataStore by preferencesDataStore("voice_leading_progress")

data class SavedConceptProgress(val conceptId: String, val competency: Competency, val evidenceCount: Int)

class ProgressStore(private val context: Context) {
    fun progress(conceptId: String): Flow<SavedConceptProgress> = context.progressDataStore.data.map { values ->
        SavedConceptProgress(
            conceptId,
            Competency.entries[values[intPreferencesKey("$conceptId.level")]?.coerceIn(0, 4) ?: 0],
            values[intPreferencesKey("$conceptId.evidence")] ?: 0,
        )
    }
    suspend fun save(progress: SavedConceptProgress) {
        context.progressDataStore.edit { values ->
            values[intPreferencesKey("${progress.conceptId}.level")] = progress.competency.ordinal
            values[intPreferencesKey("${progress.conceptId}.evidence")] = progress.evidenceCount
        }
    }
}
