package org.elmahrouss;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

/**
 * JavaFX App
 */
public class App extends Application {

    @Override
    public void start(Stage stage) {
        var javaVersion = SystemInfo.javaVersion();
        var javafxVersion = SystemInfo.javafxVersion();

        stage.setTitle("CDE");

        var labelCde = new Label("CDE");
        var labelVer = new Label(javafxVersion + ", running on Java " + javaVersion + ".");
        
        var labelPane = new VBox();
        labelPane.setSpacing(10);
        labelPane.getChildren().addAll(labelCde, labelVer);

        var scene = new Scene(labelPane, 1280, 720);
        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

}