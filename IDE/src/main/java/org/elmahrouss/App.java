package org.elmahrouss;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

/**
 * JavaFX App
 */
public class App extends Application {

    @Override
    public void start(Stage stage) {
        stage.setTitle("MetroWorks - Untitled");

        var projectPane = new VBox();
        projectPane.setSpacing(10);
        
        CodeEditor edCxx = new CodeEditor();
        projectPane.getChildren().addAll(edCxx);

        var scene = new Scene(projectPane, 1280, 720);

        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

}