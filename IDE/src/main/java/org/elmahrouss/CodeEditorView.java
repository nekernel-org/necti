/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

 package org.elmahrouss;

import java.io.Console;

import javafx.collections.ObservableList;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;

/*
 * Editor view class
 */
public class CodeEditorView extends Pane {
    private Pane linePane;
    private Label codeText;
    private boolean readOnly;
    private ConsoleWindow consoleWindow;
    private HBox codeBox;
    private String filename = "untitled.c";

    CodeEditorView(boolean readOnly) 
    {
        super();

        this.readOnly = readOnly;

        codeText = new Label();

        codeText.setStyle("-fx-font-size: 20");
        codeText.setTextFill(Color.color(1, 1, 1));

        codeText.setWrapText(true);
        codeText.setTranslateX(70);
        codeText.setTranslateY(30);

        linePane = new Pane();

        linePane.setStyle("-fx-background-color: #" + CodeEditorTheme.lineTheme);
        linePane.setMinSize(52, 720);
        linePane.setMaxSize(52, 1080);

        this.setStyle("-fx-background-color: #" + CodeEditorTheme.backgroundTheme);

        this.setMinSize(1280, 720);
        this.setMaxSize(1920, 1080);

        codeBox = new HBox();

        if (!this.readOnly) {
            consoleWindow = new ConsoleWindow();

            codeBox.getChildren().add(consoleWindow);
            codeBox.getChildren().add(codeText);
        }

        this.getChildren().addAll(linePane, codeBox);
    }

    public boolean isReadOnly() { return readOnly; }

    public void setReadOnly(Boolean readOnly) { this.readOnly = readOnly; }

    public String getFilename() { return filename; }
    
    public void setFilename(String filename) 
    { 
        if (readOnly) 
            return;
    
        this.filename = filename; 
    }

    public String getContents() { return codeText.getText(); }
    
    public void setContents(String content) 
    { 
        if (readOnly) 
            return;
    
        this.codeText.setText(content); 
    }
}
