Build the project using the default CMake preset.

Arguments: $ARGUMENTS

- If `$ARGUMENTS` is `gensln`, regenerate the Visual Studio solution and then build:
  ```bash
  cmake --preset default && cmake --build --preset default
  ```
- Otherwise, skip solution generation and just build:
  ```bash
  cmake --build --preset default
  ```

Report the output and highlight any errors or warnings.
