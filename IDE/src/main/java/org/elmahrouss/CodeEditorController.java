/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

package org.elmahrouss;

import javafx.scene.layout.Pane;

/**
 * Code Editor Controller (part of MVC arch)
 */
public class CodeEditorController 
{
    private CodeEditorView vEditorView = null;

    CodeEditorController(boolean readOnly)
    {
        vEditorView = new CodeEditorView(readOnly);
    }

    CodeEditorView getView() { return vEditorView; }
}
