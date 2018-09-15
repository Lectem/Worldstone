#pragma once

#include <MpqArchive.h>
#include "SearchableListWidget.h"

class FileBrowser
{

public:
    class IFileView
    {
    public:
        IFileView(const WorldStone::MpqArchive::Path& _filePath) : filePath(std::move(_filePath)) {}
        virtual ~IFileView();
        virtual void display() { ImGui::Text("File:%s", filePath.c_str()); }
        virtual void display(class SpriteRenderer&) { display(); }

    protected:
        WorldStone::MpqArchive::Path filePath;
    };

    ~FileBrowser();
    void display(class SpriteRenderer&);
    void displayMenuBar();

private:
    std::unique_ptr<IFileView> currentView;

    ///@name Widgets
    ///@{
    SearchableListWidget fileListWidget;
    ///@}

    ///@name Data
    ///@{
    WorldStone::IOBase::Path mpqDirectory;
    WorldStone::MpqArchive   currentArchive;

    static char const* const mpqFiles[];
    static char const* const listFiles[];
    ///@}
    void onFileSelected(const char* fileName);
};
