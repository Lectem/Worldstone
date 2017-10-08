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
