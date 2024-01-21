/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

 package org.elmahrouss;

 import javafx.collections.ObservableList;
 import javafx.scene.Node;
 import javafx.scene.control.Label;
 import javafx.scene.layout.*;

public class ConsoleWindow extends Pane 
{
    private Label vTitle = null;

    ConsoleWindow() 
    {
        super();
        
        vTitle = new Label("UNIX Console");
    }
}
