# KFF2 2.H24 browser update

H24 adds automatic alphabetical SD browsing with folders first and light-blue folder text. It fixes the blank SPECIAL listing and moves the title/F1 Help two columns left. I confirmed the browser update on my KFF2. The matching firmware source rebuilds the updater byte for byte; the current packer includes this exact updater. Existing games, saves and desktop files stay in place.

This is the Mean Hamster firmware with optional desktop startup. The [upstream C03 contribution](https://codeberg.org/r107sl/KungFuFlash2/pulls/1) includes the browser fixes and retains manual MPE launch. Prism+ renderer source and commercial game data are excluded.

[Current updater, source and notices](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/KFF2-Support.zip). Copy the updater to SD and select it through the usual KFF2 update menu. Keep your existing desktop and applications. [Hardware record](../firmware/kff2/Sys/Notices/H24-ACCEPTANCE.json). The browser confirmation does not identify every extended-CRT scenario or a complete game playthrough.
