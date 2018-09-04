#pragma once
#include <Vector.h>
#include <string>
#include "imgui/imgui_bgfx.h"

/// An imgui ListBox associated to a search filter
/// @note It will always try to fill as much space as possible vertically
class SearchableListWidget
{
    template<class T>
    using Vector = WorldStone::Vector<T>;

    Vector<std::string> elements;
    Vector<const char*> filteredElements;
    int                 currentSelectionIndex;

    ImGuiTextFilter textFilter;

    void updateFilteredElementsList();

public:
    /// Replace all the elements of the list by a new one
    void replaceElements(Vector<std::string> newElements);

    /**Display the filter and the ListBox
     * @return true if the selected element changed
     * @note   If the filter is updated, it does not trigger a selection change.
     *         This is fine because you can click on the element again to trigger the change.
     */
    bool display();

    /// @return The presently selected element. Non null if display() returned true;
    const char* getSelectedElement();
};
