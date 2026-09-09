## NeoWPF — A Modern C++ UI Framework

### Design Goals

There's a myriad of frameworks out there, and each may have their own reason to exist. NeoWPF has these design goals:

- Familiar : bring the WPF development model to native C++, including XAML, dependency properties, resources, and data binding

- Native : built around C++ from the ground up, rather than wrapping another UI framework

- Cross-platform : support multiple operating systems while keeping application code largely platform independent.

### Why Choose NeoWPF

NeoWPF aims to keep the experience and smooth API surface that WPF gives you, while also giving you native performance, many features, and improved areas where WPF is weak.
You get what WPF has, we add whatever WPF has and improve areas that are annoying to deal with instead of leaving them like that.
You don't have to deal with licensing issues, NeoWPF uses an MIT license so you don't have to worry if you use it for commercial purposes.

## Thread Safe

No need to deal with the dispatcher anymore, NeoWPF's visual tree and many other things are thread-safe, you can just update them from the background. (Though control properties are not thread-safe, and you may need to use a mutex when updating them from the background.)

### We Are Cross-platform

No need to maintain two codebases, you can have both Linux and Windows at once, without ifdefs scattered around your codebase.
We support Linux and Windows, while MacOS and Android are planned and on the way.

### Familiar API

NeoWPF is designed to closely follow WPF. It provides familiar controls, APIs, and behaviors, making it easy for WPF developers to transition without learning an entirely new framework.
XAML is one of them, you can build your apps using WPF-identical XAML in NeoWPF, no need to learn a new API surface.

### Localization

It's built in! NeoWPF makes locales a first-class member instead of just another feature bolted on top of the old features.
NeoWPF provides many localization features for you to easily make your app more widely available.
For the API, read the documentation.

### Variety of Things

NeoWPF aims to bring lots of controls from WPF, while adding new things to them as well.
If you have an idea for a new control that you think should be added, don't be afraid to leave feedback, we will read it.
We have lots of things, going from animations, data binding, triggers, virtualization, and much more for you to build your app with.

### Stability

NeoWPF is used and tested by its very own developer; I aim to test NeoWPF thoroughly and ensure that user experience is smooth and stable.
Find an issue? Tell us, and it will be fixed as soon as possible.

### Maintained

NeoWPF isn't a one-off project, I aim to update it very frequently when I have time to keep it bug free, and ensure that your experience with NeoWPF isn't annoying.