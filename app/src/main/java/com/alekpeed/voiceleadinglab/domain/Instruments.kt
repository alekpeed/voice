package com.alekpeed.voiceleadinglab.domain

enum class InstrumentFamily { GRAND, UPRIGHT, FELT, RHODES, WURLITZER }
data class EqBand(val frequencyHz: Float, val gainDb: Float, val q: Float, val bypassed: Boolean = false)
data class InstrumentPreset(
    val id: String, val name: String, val family: InstrumentFamily,
    val tone: Float, val attack: Float, val mechanicalNoise: Float,
    val tremoloDepth: Float, val drive: Float, val reverb: Float,
    val equalizer: List<EqBand>, val perNoteCents: List<Float> = List(128) { 0f },
)
object InstrumentCatalog {
    val presets = listOf(
        InstrumentPreset("grand-natural", "Concert Grand Natural", InstrumentFamily.GRAND, .55f,.55f,.08f,0f,0f,.18f,listOf(EqBand(2500f,.5f,1f))),
        InstrumentPreset("upright-warm", "Upright Warm", InstrumentFamily.UPRIGHT, .38f,.48f,.18f,0f,.03f,.12f,listOf(EqBand(220f,1.8f,.9f))),
        InstrumentPreset("felt-intimate", "Felt Intimate", InstrumentFamily.FELT, .24f,.2f,.22f,0f,0f,.25f,listOf(EqBand(2500f,-4f,.7f))),
        InstrumentPreset("rhodes-suitcase", "Rhodes Suitcase", InstrumentFamily.RHODES, .58f,.62f,.12f,.32f,.12f,.2f,listOf(EqBand(160f,1f,.8f),EqBand(3200f,2.2f,1.1f))),
        InstrumentPreset("wurlitzer-200a", "Wurlitzer 200A", InstrumentFamily.WURLITZER, .62f,.75f,.2f,.2f,.22f,.14f,listOf(EqBand(1900f,2.5f,1f))),
    )
    fun validate(preset: InstrumentPreset) = preset.id.isNotBlank() && preset.name.isNotBlank() &&
        listOf(preset.tone,preset.attack,preset.mechanicalNoise,preset.tremoloDepth,preset.drive,preset.reverb).all { it in 0f..1f } &&
        preset.perNoteCents.size == 128 && preset.perNoteCents.all { it in -100f..100f } &&
        preset.equalizer.all { it.frequencyHz in 20f..20000f && it.gainDb in -24f..24f && it.q in .1f..20f }
}
