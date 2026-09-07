## NeoWPF — A Modern C++ UI Framework

### Design Goals

There are countless of UI frameworks out there, and NeoWPF is one of them as well.
NeoWPF is built to bring the familiarity and API design from WPF to modern C++ and better features
NeoWPF aims to keep the WPF experience, while improving bad areas where WPF is weak at.

### Why Choose NeoWPF

NeoWPF aims to keep the experience and smooth API surface that WPF gives you, while also empowering you with native performance, many features, and improved areas where WPF is weak at.
You get what WPF has, we add whatever WPF has and improve areas that are annoying to deal with instead of leaving them like that.
You don't have to deal with licensing issues, NeoWPF uses an MIT license so you don't have to worry if you use it for commercial purposes.

## Thread Safe

No need to deal with the dispatcher anymore, NeoWPF's visual tree and many other things are thread-safe, you can just update them from the background. (Though control properties are not thread-safe, and you may need to use a mutex when updating them from the background)

### We Are Cross-platform

No need to maintain two codebases, you can have both Linux and Windows at once, without if defs scattered around your codebase.
We support Linux, and Windows, while MacOS and Android are planned and on the way.

### Familiar API

NeoWPF is designed to closely follow WPF. It provides familiar controls, APIs, and behaviors, making it easy for WPF developers to transition without learning an entirely new framework.
XAML is one of them, you can build your apps using WPF identical XAML in NeoWPF, no need to learn a new API surface

### Variety of Things

NeoWPF aims to bring lot's of controls from WPF, while adding new things to them as well.
If you have an idea for a new control that you think should be added, don't be afraid to leave feedback, we will read it.
We have lot's of things, going from animations, data binding, triggers, virtualization and much more things for you to build your app with.

### Stability 

NeoWPF is used and tested by it's very own developer; I aim to test NeoWPF thoroughly and ensure that user experience is smooth and stable.
Find an issue? Tell us, and it will be fixed as soon as possible.

### Maintained

NeoWPF isn't a one-off project, I aim to update it very frequently when I have time to keep it bug free, and ensure that your experience with NeoWPF isn't annoying.

