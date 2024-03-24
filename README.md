# Program for the Second Bot

This is the GitHub Repository for the Second Bot of the team WeCodeIT with the team number 0602.
This Repository contains a file "examples/secondbot/src" within it being the code of the Bot which is currently being tested and will be used for the Botball competition.
As of yet, the program is still unfinished. To make the code easier to understand, various comments explaining what a block of code will do have been added.

## Problems and Bug Fixes.
During testing, various problems, not only in the software, but also hardware occured. We will only be talking about software problems which happened during the development of the second bot.
Most of the problems originated in the Wombat. Hardships mainly arised within the consistency. Due to the Wombat being used and old, the gyro was often off, and drifts of around 0,05 - 0,1°/s were the norm.
This made coding the bot a difficulty, due to it being hard to tell, what parameter would work most often, or why it would sometimes fail, when it previously worked very consistently.
These issues were fixed using buttons, to detect when the bot would reach its destination, instead of relying on set distances.

Other problems were, trying to figure out how to incorporate various sensors and buttons to detect reaching destinations, and how to implement those within the code.
Due to the fact, that our team mainly consists of newer C++ programmers, we were not experienced in using classes, and therefore didn't exactly know how to create an instance of an object and
find a solution to calling non-static functions without first using a static function.

Thankfully the problems currently are only finding the most effective and time efficient paths to reach and fulfill a goal.

-ECER Botball team WeCodeIT
