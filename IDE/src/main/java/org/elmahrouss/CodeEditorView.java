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

/*
 * Editor view class
 */
public class CodeEditorView extends Pane 
{
    private Pane linePane;
    private Label codeText;
    private boolean readOnly;
    private ConsoleWindow consoleWindow;
    private HBox codeBox;
    private String fileName = "untitled.c";
    private CodeEditorController codeEditorController;

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

        linePane.setStyle("-fx-background-color: #" + CodeEditorTheme.LINE_THEME);
        linePane.setMinSize(52, 720);
        linePane.setMaxSize(52, 1080);

        this.setStyle("-fx-background-color: #" + CodeEditorTheme.BACKGROUND_THEME);

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

    public String getFilename() { return fileName; }
    
    public void setFilename(String fileName) 
    { 
        if (readOnly) 
            return;
    
        this.fileName = fileName; 
    }

    public String getContents() { return codeText.getText(); }
    
    public void setContents(String content) 
    { 
        if (readOnly) 
            return;
    
        this.codeText.setText(content); 
    }

    CodeEditorController getController() { return codeEditorController; }
    void setController(CodeEditorController ctrl) { codeEditorController = ctrl; }
}
