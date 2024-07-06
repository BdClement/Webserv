document.addEventListener("DOMContentLoaded", function() {
    const taskForm = document.getElementById("taskForm");
    const taskInput = document.getElementById("taskInput");
    const taskList = document.getElementById("taskList");

    taskForm.addEventListener("submit", function(event) {
        event.preventDefault();
        addTask(taskInput.value);
        taskInput.value = "";
    });

    function addTask(task) {
        if (task.trim() === "") return;

        const li = document.createElement("li");
        li.textContent = task;

        const deleteButton = document.createElement("button");
        deleteButton.textContent = "Delete";
        deleteButton.classList.add("delete");
        deleteButton.addEventListener("click", function() {
            taskList.removeChild(li);
        });

        li.appendChild(deleteButton);
        taskList.appendChild(li);
    }
});
