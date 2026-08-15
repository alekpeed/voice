package com.alekpeed.voiceleadinglab.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.LocalContext
import com.alekpeed.voiceleadinglab.domain.CurriculumCatalog
import com.alekpeed.voiceleadinglab.platform.PerformanceController

enum class Destination(val label: String, val icon: ImageVector) {
    HOME("Home", Icons.Outlined.Home), LEARN("Learn", Icons.Outlined.MenuBook),
    PRACTICE("Practice", Icons.Outlined.Piano), VISUALIZER("Voices", Icons.Outlined.Timeline),
    LAB("Lab", Icons.Outlined.Science), BARRY("Barry Harris", Icons.Outlined.GridView),
    EAR("Ear Training", Icons.Outlined.Hearing), FREE_PLAY("Free Play", Icons.Outlined.GraphicEq),
    PROGRESS("Progress", Icons.Outlined.Insights), SETTINGS("Sounds", Icons.Outlined.Tune),
}

@Composable fun VoiceLeadingLabApp() {
    var destination by remember { mutableStateOf(Destination.HOME) }
    Row(Modifier.fillMaxSize().background(MaterialTheme.colorScheme.background)) {
        NavigationRail(containerColor = MaterialTheme.colorScheme.surface) {
            Spacer(Modifier.height(12.dp))
            Destination.entries.forEach { item ->
                NavigationRailItem(selected = destination == item, onClick = { destination = item },
                    icon = { Icon(item.icon, item.label) }, label = { Text(item.label) })
            }
        }
        Column(Modifier.fillMaxSize().padding(18.dp)) {
            Text(destination.label, style = MaterialTheme.typography.headlineMedium)
            Text("Voice paths first", color = MaterialTheme.colorScheme.primary)
            Spacer(Modifier.height(14.dp))
            when (destination) {
                Destination.HOME -> HomeWorkspace()
                Destination.LEARN, Destination.PRACTICE -> StudyWorkspace(destination)
                Destination.PROGRESS -> ProgressWorkspace()
                Destination.SETTINGS -> InstrumentWorkspace()
                else -> GeneralWorkspace(destination)
            }
        }
    }
}

@Composable private fun HomeWorkspace() {
    val context = LocalContext.current
    val controller = remember { PerformanceController(context.applicationContext) }
    DisposableEffect(controller) { onDispose { controller.close() } }
    val devices = remember { controller.devices() }
    var connection by remember { mutableStateOf("Not connected") }
    ThreePane(
    { Panel("Setup") {
        if (devices.isEmpty()) Text("No USB or Bluetooth MIDI keyboard detected")
        devices.forEach { device -> Button(onClick = { controller.connect(device) { connection = if (it) "Connected" else "Connection failed" } }) {
            Text(device.properties.getString(android.media.midi.MidiDeviceInfo.PROPERTY_NAME) ?: "MIDI device")
        }}
        Text("Select instrument and velocity curve") } },
    { Panel("Continue") { Text("VL-01.1  Horizontal Hearing", style = MaterialTheme.typography.titleLarge); Button({}) { Text("Open lesson") } } },
    { Panel("Status") { Text("MIDI: $connection"); Text("Audio: Galaxy Tab output"); Text("Curriculum: 64 concepts") } },
    )
}

@Composable private fun StudyWorkspace(destination: Destination) = ThreePane(
    { Panel("Book-aligned sequence") { LazyColumn { items(CurriculumCatalog.concepts.take(16)) { Text("${it.id}  ${it.title}", modifier = Modifier.padding(vertical = 6.dp)) } } } },
    { Panel(if (destination == Destination.LEARN) "Lesson and notation" else "Exercise") { Text("Grand staff"); Spacer(Modifier.height(18.dp)); Text("Voice graph and keyboard occupy this workspace."); Spacer(Modifier.height(18.dp)); Button({}) { Text("Start MIDI capture") } } },
    { Panel("Constraints") { Text("Key: C"); Text("Voices: 2"); Text("Mode: nearest voicing"); Text("Deterministic feedback"); Slider(0.5f, {}) } },
)

@Composable private fun ProgressWorkspace() = ThreePane(
    { Panel("Concept map") { Text("64 stable VL identifiers"); Text("Prerequisites unlock at Reliable") } },
    { Panel("Competency") { listOf("Not Started", "Introduced", "Developing", "Reliable", "Fluent").forEach { Text(it, Modifier.padding(8.dp)) } } },
    { Panel("Evidence") { Text("Attempts"); Text("Unassisted successes"); Text("All-key transfer"); Text("No streaks or scores") } },
)

@Composable private fun InstrumentWorkspace() = ThreePane(
    { Panel("Instruments") { listOf("Concert Grand", "Upright Warm", "Felt Intimate", "Rhodes Suitcase", "Wurlitzer 200A").forEach { Text(it, Modifier.padding(7.dp)) } } },
    { Panel("Workshop") { Text("Tone  •  Bell  •  Bark  •  Warmth"); Slider(.55f, {}); Text("Tremolo  •  Drive  •  Reverb"); Slider(.25f, {}) } },
    { Panel("EQ and tuning") { Text("A4: 440.0 Hz"); Text("Per-note tuning"); Text("Parametric EQ"); Text("Preset save and recall") } },
)

@Composable private fun GeneralWorkspace(destination: Destination) = ThreePane(
    { Panel("${destination.label} tasks") { Text("Canonical exercises and saved sessions") } },
    { Panel("Performance") { Text("Notation, voice paths, and relevant keyboard"); Button({}) { Text("Begin") } } },
    { Panel("Analysis") { Text("Voice isolation"); Text("Slow playback"); Text("Exact factual feedback") } },
)

@Composable private fun ThreePane(left: @Composable () -> Unit, center: @Composable () -> Unit, right: @Composable () -> Unit) {
    Row(Modifier.fillMaxSize(), horizontalArrangement = Arrangement.spacedBy(14.dp)) {
        Box(Modifier.weight(.9f).fillMaxHeight()) { left() }
        Box(Modifier.weight(1.8f).fillMaxHeight()) { center() }
        Box(Modifier.weight(1f).fillMaxHeight()) { right() }
    }
}

@Composable private fun Panel(title: String, content: @Composable ColumnScope.() -> Unit) {
    Surface(Modifier.fillMaxSize(), shape = RoundedCornerShape(22.dp), color = MaterialTheme.colorScheme.surface) {
        Column(Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            Text(title, style = MaterialTheme.typography.titleMedium, color = MaterialTheme.colorScheme.primary)
            HorizontalDivider(color = MaterialTheme.colorScheme.surfaceVariant)
            content()
        }
    }
}
