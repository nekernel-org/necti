/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

 package org.elmahrouss;

import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.input.KeyCombination;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;
import javafx.scene.control.Button;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 * JavaFX App
 */
public class App extends Application {

    private static int COUNTER = 0; 

    @Override
    public void start(Stage stage) {
        stage.setTitle("MetroWorks - Untitled");
        stage.setResizable(false);

        var projectPane = new VBox();
        projectPane.setSpacing(10);

        TabPane tabePane = new TabPane();

        Tab tabEditorWelcome = new Tab("Welcome!", new CodeEditor());

        CodeEditor ed = ((CodeEditor) tabEditorWelcome.getContent());
        
        Button buttonNewPane = new Button("New project...");

        buttonNewPane.onMouseClickedProperty().set((EventHandler<MouseEvent>) (MouseEvent c) -> {
            ++COUNTER;
            Tab tabCode = new Tab("Untitled (" + Integer.toString(COUNTER) + ")", new CodeEditor());
            tabePane.getTabs().add(tabCode);
        });

        buttonNewPane.setTranslateX(70);
        buttonNewPane.setTranslateY(100);

        ed.getChildren().addAll(buttonNewPane);

        ed.setContents("Welcome to MetroWorks!\nThe embedded code editor.");
        ed.getChildren().addAll();

        tabEditorWelcome.setClosable(false);

        tabePane.getTabs().add(tabEditorWelcome);

        projectPane.getChildren().addAll(tabePane);

        var scene = new Scene(projectPane, 1280, 720);

        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

}