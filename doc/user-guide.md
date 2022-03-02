# User Guide

- [Creating a style](#creating-a-style)
- [The Style *.json file](#the-style-json-file)
  - [Common style settings](#common-style-settings)
  - [The variables section](#the-variables-section)
  - [The resources section](#the-resources-section)
  - [The icon_colors section](#the-icon_colors-section)
  - [The palette section](#the-palette-section)
- [Color themes XML files](#color-themes-xml-files)
- [Usage in QML](#usage-in-qml)

## Creating a style

To create a style, you create a folder that contains all files and resources
that are required by the Qt Advanced Stylesheet. The `qt_material` style that
comes with the library shows this. The following files are required:

- a style `*.json` file - this is them main file that describes the style (see `material.json`)
- a css template file - this file contains the stylesheet with variables that
  will be replaced during runtime (see `material.css.template`)
- a themes folder with some color themes
- a resources folder with all svg icons and images required by the css file

## The Style *.json file

### Common style settings

The style *.json file describes all style variables and settings. It starts with
some common settings:

```json
"name" : "material",
"icon" : "material_icon.svg",
"css_template" : "material.css.template",
"default_theme" : "dark_teal",
```

The `name` defines the name of th style that is returned by the `styles()`
function or that is used to set a style via `setStyle()` function. The `icon`
defines the style icon returned by the `styleIcon()` function. The `css_template`
defines the CSS template file that will be processed to get the final CSS file.
The `default_theme` defines the theme that can be set via `setDefaultTheme()`
function.

### The variables section

The variables section contains variables, that can be used in the CSS template
file. For example, the `qt_material` style defines the following variables:

```json
"variables" : {
    "font_family" : "Roboto",
    "font_size" : "12px",
    "line_height" : "12px",
    "danger" :  "#dc3545",
    "warning" : "#ffc107",
    "success" : "#17a2b8"
},
```

These variables can then be used in the style templates as follows:

```css
*{
  color: {{secondaryTextColor}};
  font-family: {{font_family}};
  line-height: {{line_height}};
  font-size: {{font_size}};
  selection-background-color: {{primaryLightColor}};
  selection-color: {{primaryTextColor}};
}

.danger{
  color: {{danger}};
}

.warning{
  color: {{warning}};
}

.success{
  color: {{success}};
}
```

To use a variable, just insert the variable name inside of two curly braces.
You can even combine a variable with an opacity value to create translucent
colors. The following example shows this. The opacity is a float value between
0 (fully transparent) and 1 (100% opaque).

```css
QStatusBar {
  color: {{secondaryTextColor}};
  background-color: {{secondaryLightColor|opacity(0.2)}};
  border-radius: 0px;
}
```

### The resources section

The resources sections defines how colors are replaced for the icons in the
resources folder that are used by the CSS style tenmplate. For each resource
set, a separate folder will be created. In the example below, two folders
are created by the style for the two resource sets `primary` and `disabled`. Both
folders contain the same svg icons files from the `resources` folder, but with
different colors. That means, you can create a set of normal, disabled or hoverd
icons with different colors unsing the resources section.

```json
"resources" : {
    "primary" : {
        "#0000ff" : "primaryColor",
        "#ff0000" : "secondaryColor",
        "#000000" : "#ffffff00"
    },
    "disabled" : {
        "#0000ff" : "secondaryLightColor",
        "#ff0000" : "secondaryColor",
        "#000000" : "#ffffff00"
    }
},
```

The resources can then be used in the CSS style template as follows. You can see,
how the `disabled` and `primary` resource sets are used:

```css
QTabBar QToolButton::up-arrow {
  image: url(icon:/disabled/uparrow2.svg);
}

QTabBar QToolButton::up-arrow:hover {
  image: url(icon:/primary/uparrow2.svg);
}

QTabBar QToolButton::down-arrow {
  image: url(icon:/disabled/downarrow2.svg);
}

QTabBar QToolButton::down-arrow:hover {
  image: url(icon:/primary/downarrow2.svg);
}
```

In each set, you define which color values in the SVG file should be replaced
with a certain color from the style. The following definition for the `primary`
resource set means the following:

- the SVG color `#0000ff` will be replaces with the color defined in the theme 
  color variable `primaryColor`
- the SVG color `#ff0000` will be replaced by the theme color `secondaryColor` and
- the SVG color `#000000` will be replaced by the color `#ffffff00`

```json
"primary" : {
      "#0000ff" : "primaryColor",
      "#ff0000" : "secondaryColor",
      "#000000" : "#ffffff00"
 },
```

That means, you can use color variables and fixed color values in this section.

### The icon_colors section

The `icon_colors` section defines a list of colors that are replaced in SVG
icons that are loaded via the function `loadThemeAwareSvgIcon()`:

```json
"icon_colors" : {
    "#03b8e5" : "secondaryTextColor"
 },  
```

This definition means, that the SVG color `#03b8e5` will be replaced by the
color defined by the theme color variable `secondaryTextColor`.

### The palette section

This section defines the palette colors that will be set by the style. You can
define the colors for different palette color roles (`Disabled`, `Active`, `Inactive`,
`Normal`).

```json
"palette" : {
    "active" : {
        "Window" : "",
        "WindowText" : "secondaryTextColor",
        "Base" : "",
        "AlternateBase" : "",
        "ToolTipBase" : "",
        "ToolTipText" : "",
        "PlaceholderText" : "",
        "Text" : "secondaryTextColor",
        "Button" : "",
        "ButtonText" : "secondaryTextColor",
        "BrightText" : "",
        "Light" : "secondaryColor",
        "Midlight" : "",
        "Dark" : "",
        "Mid" : "",
        "Shadow" : "",
        "Highlight" : "",
        "HighlightedText" : ""
    },

    "inactive" : {

    },

    "disabled" : {

    }
}
```

## Color themes XML files

The themes folder contains color themes XML files, that define the colors for
the color variables used in the style template. The following theme file defines
the colors for the `qt_material` theme `dark_theme`. The theme defines the
color variables and its color values.

Additionally, a the should define the `dark` attribute as 0 or 1 to define
if this is a dark theme or not. If this is properly defined, you can use
the function `isCurrentThemeDark()` to find out, if the current theme is
a dark theme.

```xml
<!--?xml version="1.0" encoding="UTF-8"?-->
<resources dark="1">
  <color name="primaryColor">#1de9b6</color>
  <color name="primaryLightColor">#6effe8</color>
  <color name="secondaryColor">#232629</color>
  <color name="secondaryLightColor">#4f5b62</color>
  <color name="secondaryDarkColor">#31363b</color>
  <color name="primaryTextColor">#000000</color>
  <color name="secondaryTextColor">#ffffff</color>
</resources>
```

## Usage in QML

This project can also be used with QML applications. In addition to the steps 
described in the [previous paragraph](#getting-started) you need to register the 
provided `CQmlStyleUrlInterceptor` to the QML Engine you're using.

Let's say you have your `CStyleManager` instance and a `QQuickWidget` that 
displays your QML content. The only thing you need to do now is the following:

```cpp
acss::CStyleManager* StyleManager = new acss::CStyleManager;

QQuickWidget Widget;
Widget.engine()->setUrlInterceptor(new CQmlStyleUrlInterceptor(StyleManager));
```

And that's it. Now you can use all of the icons provided by the style manager as
you would in your C++ code:

```qml
CheckBox {
    id: checkBox

    indicator: Rectangle {
        implicitHeight: 26
        implicitWidth: 26
        x: checkBox.leftPadding
        y: checkBox.height / 2 - height / 2
        Image {
            source: checkBox.checked ? "icon:/primary/checkbox_checked.svg" :
                                        "icon:/primary/checkbox_unchecked.svg"
            // Important: Disable caching because otherwise you won't see any changes
            cache: false
        }
    }
}
```

Check the `full_features` example to see this in action.
