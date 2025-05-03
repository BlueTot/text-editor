# Text Editor

A (work in progress) extension of the _kilo_ text editor

## License

This project is based on [kilo](https://github.com/antirez/kilo) by Salvatore Sanfilippo.

Original code is licensed under the [BSD 2-Clause License](https://opensource.org/licenses/BSD-2-Clause). This repository retains the same license unless otherwise stated.

Modifications and additions are my own work.

---

### Attribution

Kilo text editor was originally created by [@antirez](https://github.com/antirez). See the [original kilo repository](https://github.com/antirez/kilo) for the source and full license details.

---

### Additional features

- Vim-like modes

  - Normal mode: Move around document, save and quit
  - Insert mode: Insert text
  - Visual mode: Select text
  - Visual Line mode: Select lines of text

- Vim-like macros
  - `a`: Insert after cursor
  - `i`: Insert before cursor
  - `h`,`j`,`k`,`l`: Move left, down, up, right
  - `v`: Visual mode
  - `<esc>`: Go to normal mode
  - `g`: Top of file
  - `G`: Bottom of file
  - `0`: Start of line
  - `$`: End of line
  - `y`: Yank (copy) text into buffer
  - `p`: Paste text from buffer
