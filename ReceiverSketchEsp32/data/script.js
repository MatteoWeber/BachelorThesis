let updateInterval = null;

//function to set the application
function selectApplication() {
  const application = document.getElementById("application").value;

  // Make a request to select the application
  const xhr = new XMLHttpRequest();
  xhr.open("GET", `/select?application=${application}`, true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      // Update the status text
      document.getElementById("status").innerText = xhr.responseText;

      // Show application stats card
      const statsCard = document.getElementById("application-stats");
      statsCard.style.display = "block";
    } else {
      document.getElementById("status").innerText = "Error selecting application.";
    }
  };
  xhr.send();
}

//function to start the application
function startInfoUpdates() {
  // Clear any existing intervals
  if (updateInterval) {
    clearInterval(updateInterval);
  }
  // Set an interval to update the application stats every 200 ms
  updateInterval = setInterval(fetchInfo, 200);
}

//function to stop the application
function stopInfoUpdates() {
  // Stop the interval that fetches application info
  if (updateInterval) {
    clearInterval(updateInterval);
    updateInterval = null;
  }
}

//function to retrieve the cube data from the /info endpoint
function fetchInfo() {
  const xhr = new XMLHttpRequest();
  xhr.open("GET", "/info", true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      const data = xhr.responseText.split("\n");
      const cubeData = {
        "Cube 1": {},
        "Cube 2": {},
        "Cube 3": {},
      };

      data.forEach((line) => {
        // Split line into parts
        const parts = line.split(", ");

        // Skip lines that do not contain valid data (e.g., if it's an empty line or header)
        if (parts.length === 4) {
          // Parse and clean up the "Top Face" part (remove the "Top Face: " label)
          const label = parts[0].trim();
          const accelX = parts[1].trim();
          const accelY = parts[2].trim();
          const accelZ = parts[3].trim();

          if (label.startsWith("Cube 1 Data:")) {
            // Clean "Top Face" label
            const topFace = label.replace("Cube 1 Data: Top Face: ", "");
            cubeData["Cube 1"] = { topFace, accelX, accelY, accelZ };
          } else if (label.startsWith("Cube 2 Data:")) {
            // Clean "Top Face" label
            const topFace = label.replace("Cube 2 Data: Top Face: ", "");
            cubeData["Cube 2"] = { topFace, accelX, accelY, accelZ };
          } else if (label.startsWith("Cube 3 Data:")) {
            // Clean "Top Face" label
            const topFace = label.replace("Cube 3 Data: Top Face: ", "");
            cubeData["Cube 3"] = { topFace, accelX, accelY, accelZ };       
          } 
        }
      });

      // Dynamically update the data cells with the fetched data
      document.getElementById("cube-1-topface").innerText =
        cubeData["Cube 1"].topFace || "N/A";
      document.getElementById("cube-2-topface").innerText =
        cubeData["Cube 2"].topFace || "N/A";
      document.getElementById("cube-3-topface").innerText =
        cubeData["Cube 3"].topFace || "N/A";

      document.getElementById("cube-1-accelX").innerText =
        cubeData["Cube 1"].accelX || "N/A";
      document.getElementById("cube-2-accelX").innerText =
        cubeData["Cube 2"].accelX || "N/A";
      document.getElementById("cube-3-accelX").innerText =
        cubeData["Cube 3"].accelX || "N/A";

      document.getElementById("cube-1-accelY").innerText =
        cubeData["Cube 1"].accelY || "N/A";
      document.getElementById("cube-2-accelY").innerText =
        cubeData["Cube 2"].accelY || "N/A";
      document.getElementById("cube-3-accelY").innerText =
        cubeData["Cube 3"].accelY || "N/A";

      document.getElementById("cube-1-accelZ").innerText =
        cubeData["Cube 1"].accelZ || "N/A";
      document.getElementById("cube-2-accelZ").innerText =
        cubeData["Cube 2"].accelZ || "N/A";
      document.getElementById("cube-3-accelZ").innerText =
        cubeData["Cube 3"].accelZ || "N/A";
    } else {
      // If there's an error fetching the data, display a message
      document.getElementById("stats-content").innerText =
        "Error fetching application info.";
    }
  };
  xhr.send();
}

//function to load the application dynamically into the dropdown menu
function loadApplications() {
  const xhr = new XMLHttpRequest();
  xhr.open("GET", "/applications", true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      const applications = JSON.parse(xhr.responseText);

      // Get the dropdown element
      const dropdown = document.getElementById("application");
      dropdown.innerHTML = ""; // Clear any existing options

      // Populate the dropdown with the applications
      applications.forEach((application) => {
        const option = document.createElement("option");
        option.value = application.id;
        option.textContent = application.name;
        dropdown.appendChild(option);
      });
    } else {
      console.error("Failed to load application.");
    }
  };
  xhr.send();
}
