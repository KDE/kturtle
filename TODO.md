
 = kturtle meeting 26oct09 (niels, cies) =

- QTextDocument to replace QString in the tokenizer for full unicode support
- DBUS interface for SVG
- ask HIG/celeste about 'Switch application Language'
- cmd language selection _only_ as commandline option


 = KTurtle TODOs =

Besides this document you can find TODO items in the code (try grepping for 'TODO'), in the BUGS file and on bugs.kde.org.

 == general ==
 * complain about the 'Switch Application Language...' in the Help menu (help center always uses system language)
 * Run action group has WhatIsThis, but it doesnt work -- file a bug...
 * no secret that i dont like WhatIsThis, it cannot even refer by a link to the Handbook! (useless piece of s***)
 * dowload some other LOGO programming manuals that are targetted at young crowd, use these while editing the handbook

 == mainwindow ==
 * have a look at KDE's new build-in 'Switch Application Language...', could it fully replace ours.
 * make fullscreen mode (hide all GUI elements but the canvas during execution)
 * printing dialogs may need some finetuning (like: further kdeification, overwrite protection, etc.)
 * do reordering (in stead of duping) when adding already existing recent file items
 * disable most of the actions (like File, Edit, Canvas and more) while executing (BUG in bugzilla)
 * clean up export methods: make all KUrl aware, and make the dialog generic
 * split the context help functionality out of mainwindow

 == interpreter ==
 * finish unittests
 * fix failing unittests (division by zero, sqrt of a negtive)
 * maybe make some switch statement to flip between DEG and RAD
 * profile the whole thing... just curiosity -- how much time is spend in what functions
 * commandline invoke doesnt handle local files well (like: ../script.turtle)
 * make state of more things available to the executer
    - add get* commands (get_penwidth or getpenwidth)
    - allow argumment over(under)loading, $pw = penwidth
    - make state avalable as variables ($_penwidth, $_x, $_x = 100, etc)
    - one get command that takes a string (get "penwidth")

 == canvas ==
 * context menu (right click) for export and print
 * DON'T add wrapping functionality (not a ToDo, but a DontDo)
 * GraphicsView not as deterministic in painting as a pixel raster (double lines look more thick)

 == editor ==
 * finish context help (F2) action -- work on the handbook
 * disable save (not saveAs), or make it work properly for examples
 * save can be disabled after a code has been saved

 == handbook ==
 * maybe a section on parenthesis and/or scopes
 * spellchecking

 == PR ==
 * make wikipedia page nicer (screenshots, etc.)
 * release a pre-release
 * maybe: team up with windows/osx team for a kturtle only installers
 * a page (wiki?) for childrens and teachers manual + translations + translators info [or docbook; KDE translated] (userbase??)
 * get someone with some english writing/ marketing writing skills to write a small, medium and long intro to kturtle
 * use these new writings in: handbook, main.cpp, kde-apps, wikipedia, edu.kde.org/kturtle

 == website ==
 * move all screenies to userbase

 == maybe someday's ==
 * refactor the generators ruby code
 * create a generated parser (yacc/bison style; Qt has a parser generator, KDevelop has one too)




PLAN: SOLVE GENERAL AMBIGUITY REGARDING THE TOKEN thingy
right now token types are used all over the place. this is not the right way because:
 - Token types should be used for tokenizing of the string only. (ie: fw,{,"s",$a, ,#littlecomment,2.2,EOL,+,= are tokens)
 - right now their are also used as TreeNode types (ie: Scope, Root, ArgumentList)
 - and they are used as translatables in the translation lookup table (especially $,.,# are not tokens by them selves)
currently this is not really a problem -- it works. yet it is not 'how it should be'.
fixing this involves a big rewrite of the generator code, which result may be a major refactor of the interpreter.
