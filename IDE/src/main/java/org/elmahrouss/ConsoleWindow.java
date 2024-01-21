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
import javafx.scene.paint.Color;

public class ConsoleWindow extends Pane 
{
    private Label vTitle = null;

    ConsoleWindow() 
    {
        super();
        
        vTitle = new Label("Debug Output:");

        vTitle.setStyle("-fx-font-size: 20");
        vTitle.setTextFill(Color.color(1, 1, 1));

        vTitle.setTranslateX(80);
        vTitle.setTranslateY(500);

        this.getChildren().add(vTitle);
    }
}
