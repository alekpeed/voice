package com.alekpeed.voiceleadinglab.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val Nocturne = darkColorScheme(
    primary = Color(0xFF8DD8CA), secondary = Color(0xFFB8A9EF), tertiary = Color(0xFFE8B875),
    background = Color(0xFF0D1117), surface = Color(0xFF151B24), surfaceVariant = Color(0xFF202938),
    onPrimary = Color(0xFF08201C), onBackground = Color(0xFFE8EEF5), onSurface = Color(0xFFE8EEF5),
)

@Composable fun VoiceLeadingLabTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = Nocturne, content = content)
}
