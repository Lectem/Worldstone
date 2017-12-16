12/16/2017
- Trying to choose a library or API for rendering
  * No to https://github.com/floooh/oryol as it doesn't have shader hot reloading
  * It needs either Metal or OpenGL for OSX...
  * Try either BGFX or Magnum

11/26/2017
- Use CHECK and == for unit test instead of CHECK_EQ when possible
  * Visually easier to understand, we're used to see ==
  * Some libraries do not provide CHECK_EQ equivalents (harder to switch if needed)
  * Can still use CHECK_FALSE : easier to see than just using CHECK(!expression)
  * In some cases doctest can't evaluate the expressions, so still have to use CHECK_EQ...

09/18/2017

- Not use the GSL
  * Creates too many warnings (conversion, weaktables...)
  * Interface does not match the standard library (size() returns signed values)
  * Might need to re-evaluate this decision later
  * Some performance concerns

12/08/2016

- Drop QML
  * Not meant for tools
  * Missing docking widgets
  * Missing tree views and a lot of stuff
  * Need to convert every POD to a Q_GADGET with properties
  * C++/QML under/badly-documented
    - Why not say explicitely that you can't pass by value/reference in signals from c++
    - Most of the stuff is implicitly converted to a QVariant
    - QML "id:" with custom type is broken, you need to use properties in QML to change values
  * So on and so forth...
- Don't use Qt Designer .ui
  * Because .xml
  * A pain to review
  * Perhaps for prototyping only ?
