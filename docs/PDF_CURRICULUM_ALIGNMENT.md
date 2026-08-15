# PDF Curriculum Alignment

The three study PDFs in `Voice_Leading_and_Barry_Harris_PDFs.zip` are the
authoritative curriculum sources for Voice Leading Lab. The app must not create
a competing chapter sequence or rename a published concept.

## Canonical identity

- Volume I concept IDs (`VL-01.1` through `VL-16.4`) are the primary keys for
  voice-leading progress, exercises, analytics, and book links.
- Volume II units are identified as `VL-U01` through `VL-U16`; its printed
  exercises are identified as `VL-EX-01A` through `VL-EX-16E`.
- Volume II etudes are identified as `VL-ETUDE-01` through `VL-ETUDE-12`.
- Barry Harris guide chapters are identified as `BH-CH-01` through `BH-CH-33`;
  appendices use `BH-APP-A` through `BH-APP-E`.

The `VL-U*`, `VL-EX-*`, `VL-ETUDE-*`, `BH-CH-*`, and `BH-APP-*` forms are app
identifiers for book structures that do not already publish their own stable
IDs. The visible labels and titles remain those printed in the PDFs.

## Edition metadata

`CurriculumCatalog` records the exact PDF filename, SHA-256 digest, page count,
and current physical start pages. Page numbers are navigation metadata only.
They are not identity: the Volume I book itself directs readers to use concept
IDs because pagination may change.

| Source | Indexed structure |
|---|---|
| Volume I Study Guide | 16 chapters, 64 concepts |
| Volume II Practice Companion | 16 matching units, 80 exercises, 12 etudes |
| Moving Harmony / Barry Harris Guide | 5 parts, 33 chapters, 5 appendices |

## Progress-scale mapping

The Companion's 0–4 benchmark language maps to the app's competency language
without changing the underlying order.

| Book level | Book label | App competency |
|---:|---|---|
| 0 | Unknown | Not Started |
| 1 | Recognized | Introduced |
| 2 | Controlled | Developing |
| 3 | Transferable | Reliable |
| 4 | Musical | Fluent |

## Implementation rule

Each generated or interactive activity must reference at least one canonical
concept or chapter ID. Printed Volume II exercises remain distinct from
generated app exercises, even when both train the same concepts. The catalog
stores book titles and routing metadata; the complete instructions, notation,
and prose remain in the included PDFs.

Automated tests enforce source counts, unique IDs, complete cross-volume
coverage, five printed exercises per unit, Barry Harris chapter coverage, and
the competency mapping.
