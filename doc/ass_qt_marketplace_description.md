# Qt Advanced Stylesheets

Advanced Stylesheets with Theming Support for Qt.

The library allows runtime color switching for CSS stylesheet themes including
SVG resources and SVG icons. The image below shows switching of accent color
and switching between dark and light theme. Notice how the icons in the left
sidebar change color when switching between dark and light theme.

![color_switching](https://raw.githubusercontent.com/githubuser0xFFFF/Qt-Advanced-Stylesheets/master/doc/CETONI_Elements_Styling.gif)

## Features

The main features are:

- runtime switching of CSS colors
- runtime color switching of CSS SVG icons / resources
- runtime color switching of theme aware icons loaded via `loadThemeAwareSvgIcon()`
- runtime switching of QPalette colors
- definition of CSS styles that switch the complete application design
- definition of XML color themes that allow switching of theme colors (dark / light)
- switching of individual theme color or switching of accent color
- QML support

If you run the `full_features` example, then you can test the functionality.
There are some custom dark themes:

![dark](https://raw.githubusercontent.com/githubuser0xFFFF/Qt-Advanced-Stylesheets/master/doc/qt_material_dark.gif)

And light:

![light](https://raw.githubusercontent.com/githubuser0xFFFF/Qt-Advanced-Stylesheets/master/doc/qt_material_light.gif)

The library allows you to create stylesheets that contain variables that are
replaced at runtime like in this example:

```css
QComboBox:disabled {
  color: {{primaryColor|opacity(0.2)}};
  background-color: {{secondaryColor|opacity(0.75)}};
  border-bottom: 2px solid {{primaryColor|opacity(0.2)}};
}

QComboBox::drop-down {
  border: none;
  color: {{primaryColor}};
  width: 20px;
}
```

## Theme-aware Icons

The library supports loading of theme-aware SVG icons.
This allows runtime color switching of application icons like in the
toolbar image below.

![Theme Aware Icon](https://raw.githubusercontent.com/githubuser0xFFFF/Qt-Advanced-Stylesheets/master/doc/theme_aware_icons.gif)
