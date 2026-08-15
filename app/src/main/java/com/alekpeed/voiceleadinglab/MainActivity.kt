package com.alekpeed.voiceleadinglab

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.alekpeed.voiceleadinglab.ui.VoiceLeadingLabApp
import com.alekpeed.voiceleadinglab.ui.theme.VoiceLeadingLabTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent { VoiceLeadingLabTheme { VoiceLeadingLabApp() } }
    }
}
