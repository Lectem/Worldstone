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

    static constexpr const char* mpqFiles[] = {"d2char.mpq",  "d2data.mpq",  "d2exp.mpq",
                                               "d2music.mpq", "d2sfx.mpq",   "d2speech.mpq",
                                               "d2video.mpq", "d2xtalk.mpq", "d2xvideo.mpq"};
    static constexpr const char* listFiles[] = {"listfile.txt"};
    ///@}
    void onFileSelected(const char* fileName);
};
