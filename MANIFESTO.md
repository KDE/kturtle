# KTURTLE MANIFESTO

KTurtle is an educational programming environment that aims to make programming as easy and touchable as possible. It is not intended to be a general purpose programming language, but merely helps a student who is new to programming to quickly get a basic understanding.

## Why not use an existing programming language to teach?

Many programming languages exist, yet usually require the programmer to have a basic understanding of English. Usually it is even more than a basic understanding that is needed, the terminology used in the error messages of a programming language can be quite difficult.

Furthermore one finds that many programming languages come with a variety of separate tools and often require understaning of a command shell or IDE (integrated development environment) in order to used. This raises the entry barrier for someone new to programming as a lot has to be learned in order to see the first results.

KTurtle is one integrated program, and is _fully_ transaltable into the native language of the student. KTurtle is not the first educational programming language, there are some implementations of the LOGO programming language with comparable features. Yet most of these products are not freely available, not available on the main platforms (Linux, Windows and OSX), miss translations, are not actively maintained.

Specific features of KTurtle:
 * highlighting during execution (shows what the executer is executing)
 * highlighting of errors
 * context help (hit F2 for help on the piece of script right under the cursor)
 * fully translatable (the programming language itself, the examples, the error messages, the GUI, the manual... everything)
 * yet still RealCoding(tm), no clicky clicky stuff
 * and everything in one program (instead of separate: debugger, documentation, IDE, interpreter, etc.)

## Why do you not implement LOGO?

KTurtle started off with a syntax that was more close to LOGO. Many complaints arose claiming KTurtle was not LOGO enough (doing things different, missing features, or just not being 100% compliant). While there is no such thing as a LOGO standard, most refered to UCB-LOGO as the defacto standard.

To solve this issue Cies Breijs, the initiator off the project, decided to move away from the LOGO'ish syntax to a syntax that is more similar to commonly used programming languages like C, C++, Java, PHP, Perl, Python, Ruby, C#, VB and ASP. This will make it a little easier for a student to move from KTurtle to general purpose programming languages.

## Where does the turtle come from?

Some versions of LOGO shipped with (something similar to) Turtle-Graphics. This is a set of commands to move a turtle around and use it to make drawings. The initiator of KTurtle, Cies Breijs, learned programming using MSX-LOGO which also featured a turtle. As a reference to the roots of KTurtle (LOGO and Turtle-Graphics) the turtle has been kept as a symbol of educational programming.









