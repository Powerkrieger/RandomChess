# Shogun (RandomChess)

Android version of Ravensburger's *Shogun* (1979), the magnetic board game where every piece
carries a number that decides how far it moves. Native C++ / OpenGL ES app built on the
Android Game Activity template, with a computer opponent.

## Rules

- 8x8 board, each side has a king and seven soldiers on its home rank.
- Every piece shows a number: soldiers 1–4, the king 1–2. A piece moves **exactly** that many
  squares orthogonally, in a straight line or with a single 90° turn. It cannot jump; every
  square on the path except the last must be empty. The last square may hold an enemy piece,
  which is captured.
- After a piece moves it gets a new random number.
- You win by capturing the enemy king or reducing the enemy to fewer than three pieces.

## Layout

Single Gradle module (`app`). The Kotlin side is only the `GameActivity` shell; everything else is
C++ under `app/src/main/cpp`:

| Path | Contents |
| --- | --- |
| `chess/` | Platform-independent game logic. `Piece` (movement primitives `slide`, `step`, `walk`), the concrete pieces, `Board`, `Rules` (`ChessRules`, `ShogunRules`), `Game` (turns, selection, history) and `ShogunAi` (expectiminimax with a chance node for the re-roll). No Android dependencies — fully unit-tested on the host. |
| `Stats.*` | Results history (wins/losses per side and difficulty, starting numbers of every game) persisted as a TSV file in the app's internal storage. |
| `App.*` | Screens (menu, game with move list, results history), settings and the computer opponent driving. Renderer-independent. |
| `Canvas.*`, `BoardView.*` | Immediate-mode 2D drawing: quads, sprites, bitmap-font text, the board. |
| `Renderer.*`, `Shader.*`, `TextureAsset.*`, `main.cpp` | EGL/GLES setup, touch input and the app loop from the Game Activity template. |

Art in `app/src/main/assets` (stones, digits, chess sprites, font) is generated placeholder art
made with ImageMagick from the DejaVu fonts.

## Building

Standard Gradle/Android Studio project with the NDK. Open it in Android Studio, or from the CLI:

```
./gradlew assembleDebug
```

The game-logic tests don't need the NDK or a device:

```
cmake -S app/src/test/cpp -B build/cpp-tests
cmake --build build/cpp-tests
ctest --test-dir build/cpp-tests --output-on-failure
```

## Release builds & CI

`.github/workflows/build.yml` runs the C++ unit tests and builds a signed release APK on every
push to `main` and on version tags (`v*`), attaching the APK to a GitHub Release for tagged
builds. Signing reads from environment variables (`KEYSTORE_PATH`, `KEYSTORE_PASSWORD`,
`KEY_ALIAS`, `KEY_PASSWORD`), populated in CI from repo secrets. Same setup as `NFLunkyBall`,
`QuickMusicQuiz` and `AmelieMusikster`.

To build a signed release locally, set those four environment variables (pointing
`KEYSTORE_PATH` at your own `.jks` file) and run `./gradlew assembleRelease`.

## AI notice

Project scaffolding, CI pipeline and large parts of the app were built with Claude Code.
