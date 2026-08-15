package com.alekpeed.voiceleadinglab.platform

import android.content.Context
import java.io.File

enum class StudyBook(val assetName: String) {
    VOICE_LEADING_I("Voice_Leading_for_Pianists_Volume_I_Study_Guide_Notation_Fixed.pdf"),
    VOICE_LEADING_II("Voice_Leading_for_Pianists_Volume_II_Practice_Companion_Notation_Fixed.pdf"),
    BARRY_HARRIS("Barry_Harris_Piano_Method_Independent_Study_Guide.pdf"),
}

class BookAssets(private val context: Context) {
    fun materialize(book: StudyBook): File {
        val directory = File(context.filesDir, "books").apply { mkdirs() }
        val destination = File(directory, book.assetName)
        if (!destination.exists()) {
            context.assets.open("books/${book.assetName}").use { input ->
                destination.outputStream().use(input::copyTo)
            }
        }
        return destination
    }
}
