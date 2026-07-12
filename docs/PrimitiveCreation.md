## How to properly create a UI primitive

First of all, a ui primitive typically consists of 4 files (depending on what ui primitive you're working with)
-The main file
-The styling file (if your primitive needs it)
-The template file (if your primitive needs it)
-The states file (if your primitive needs it)
-And any other file that you desire or need to create.

for example, you have the button UI primitive, which consists of:
-Button.cpp
-ButtonStates.cpp
-ButtonStyles.cpp
-ButtonTemplate.cpp
-ButtonProperties.cpp

Folder structure would be:
Button/Button.cpp
Button/Properties.cpp
Button/Visuals/ButtonStyles.cpp
Button/Visuals/ButtonTemplate.cpp
Button/Visuals/ButtonStates.cpp
Button/Behavior (but button doesnt need this since it uses ButtonBase)

Make sure that the namespace for the .Styles will be exactly as "NeoWPF::UI::Styling"
and for Template and the main file itself under "NeoWPF::UI"

An UI primitive must either exist under:
-NeoWPF\src\UI\Controls (if it's an UI element, something that the user interacts with)
-NeoWPF\src\UI\Layout (if it's only for layout, like panel or grid)

Styles and Template files must be added under the primitive's own folder, like this:
NeoWPF\src\UI\Controls\Button
You put in there Button.cpp, Button.Styles.cpp and ButtonTemplate.cpp using the ControlTemplate implementation
and default styles implementation.

Also, use ContentControl (Content) and Presenter for UI controls!!

For a ControlTemplate, you can begin by defining required and non required parts for the template to be valid.
For default styles simply specify the default colors of the UI element as the minimum.
For states, define the commons states that your primitive can have and add the default colors for them in styles

After you have created all these files and you are sure that NeoWPF builds, proceed to do this after:
-Commit the change using Git using this template "Feat: Added base implementation for {name}

After that, proceed by adding the UI primitive to UI.h, then go inside NeoWPF.h and add it there. You should also
try and document that UI primitive, by following the rules inside CommentingRules.md (very important) and the actual
formatting rules inside CommentFormatting.md so they are properly written.
After that, you will then proceed to do another Git commit with the following template "Chore: Added {name} to the public API"