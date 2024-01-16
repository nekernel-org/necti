/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

 package org.elmahrouss;

import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.event.EventTarget;
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
public class App extends Application 
{
    @Override
    public void start(Stage stage) 
    {
        stage.setTitle("MetroWorks - v1.00");
        stage.setResizable(false);

        var projectPane = new VBox();
        projectPane.setSpacing(10);

        TabPane tabPages = new TabPane();

        Tab tabEditorWelcome = new Tab("Welcome!", new CodeEditorView(false));

        CodeEditorView editorView = (CodeEditorView)tabEditorWelcome.getContent();
        
        Button buttonNewPane = new Button("Add...");

        buttonNewPane.onMouseClickedProperty().set((EventHandler<MouseEvent>) (MouseEvent c) -> {
            CodeEditorController view = new CodeEditorController(true);
            Tab tabCode = new Tab("Untitled", view.getView());

            view.getView().setController(view);

            tabCode.setText(view.getView().getFilename());

            tabPages.getTabs().add(tabCode);
        });

        buttonNewPane.setTranslateX(70);
        buttonNewPane.setTranslateY(100);

        editorView.getChildren().addAll(buttonNewPane);

        editorView.setContents("Welcome to MetroWorks!\nThe embedded code editor.");
        editorView.getChildren().addAll();

        tabEditorWelcome.setClosable(false);

        tabPages.getTabs().add(tabEditorWelcome);

        projectPane.getChildren().addAll(tabPages);

        var scene = new Scene(projectPane, 1280, 720);

        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

}