package org.elmahrouss;

import javafx.beans.property.ObjectProperty;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.*;

public class CodeEditor extends Pane {
    private Pane linePane;
    private Label lines;

    CodeEditor() {
        super();

        lines = new Label();
        
        lines.setStyle("-fx-font-size: 20");
        lines.setWrapText(true);
        lines.setTranslateX(70);
        lines.setTranslateY(30);

        linePane = new Pane();

        linePane.setStyle("-fx-background-color: #" + CodeEditorTheme.lineTheme);
        linePane.setMinSize(52, 720);
        linePane.setMaxSize(52, 1080);

        this.getChildren().addAll(linePane, lines);

        this.setStyle("-fx-background-color: #" + CodeEditorTheme.backgroundTheme);

        this.setMinSize(1280, 720);
        this.setMaxSize(1920, 1080);
    }

    @Override
    public ObservableList<Node> getChildren() {
        return super.getChildren();
    }

    public String getContents() { return lines.getText(); }
    public void setContents(String content) { lines.setText(content); }
}
