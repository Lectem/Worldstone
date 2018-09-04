#pragma once

#include <MpqArchive.h>
#include "SearchableListWidget.h"

class FileBrowser
{

public:
    class IFileView
    {
    public:
        virtual ~IFileView();
        virtual void display() {}
        virtual void display(class SpriteRenderer&) { display(); }
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
