#include "SearchableListWidget.h"

void SearchableListWidget::replaceElements(Vector<std::string> newElements)
{
    elements = std::move(newElements);
    filteredElements.clear();
    filteredElements.reserve(elements.size());
    for (const std::string& str : elements)
        filteredElements.push_back(str.data());
    currentSelectionIndex = 0;
    textFilter.Clear();
}
void SearchableListWidget::updateFilteredElementsList()
{
    filteredElements.clear();
    for (const std::string& str : elements)
    {
        if (textFilter.PassFilter(str.data())) filteredElements.push_back(str.data());
    }
}

bool SearchableListWidget::display()
{
    if (textFilter.Draw("##Filter")) {
        updateFilteredElementsList();
    }
    if (!filteredElements.size()) {
        ImGui::Text("No file found.");
        return false;
    }
    return ImGui::ListBox(
        "##List", &currentSelectionIndex, filteredElements.data(), int(filteredElements.size()),
        int(ImGui::GetContentRegionAvail().y / ImGui::GetTextLineHeightWithSpacing()) - 1);
}

const char* SearchableListWidget::getSelectedElement()
{
    if (currentSelectionIndex >= 0 && size_t(currentSelectionIndex) < filteredElements.size()) {
        return filteredElements[size_t(currentSelectionIndex)];
    }
    return nullptr;
}
