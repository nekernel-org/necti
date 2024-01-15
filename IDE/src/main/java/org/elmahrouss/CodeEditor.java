package org.elmahrouss;

import javafx.scene.layout.*;

public class CodeEditor extends Pane {
    CodeEditor() {
        super();

        this.setStyle("-fx-background-color: #" + CodeEditorTheme.backgroundTheme);
        this.setMinSize(1280, 720);
        this.setMaxSize(1920, 1080);
    }
}
