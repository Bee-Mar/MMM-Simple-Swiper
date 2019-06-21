const NodeHelper = require("node_helper");
const path = require("path");

module.exports = NodeHelper.create({
  socketNotificationReceived: (notification, payload) => {
    const threshold = payload["threshold"];
    const distDiff = payload["distanceDiff"];

    const DEBUG = payload["debug"];

    // creating demon spawn
    const child = require("child_process").spawn("sudo", [
      path.join(__dirname, "/mmm_simple_swiper"),
      JSON.stringify(payload)
    ]);

    // using arrow function to keep 'this' reference of outer scope
    child.stdout.on("data", (data) => {
      data = data
        .toString()
        .replace(/\s+/g, "")
        .trim()
        .split(":");

      const leftDist = parseFloat(data[0]).toFixed(0);
      const rightDist = parseFloat(data[1]).toFixed(0);

      if (DEBUG) {
        console.log("LEFT: " + leftDist + ", RIGHT: " + rightDist);
      }

      if (leftDist <= threshold && rightDist <= threshold) {
        if (leftDist * distDiff <= rightDist) {
          this.sendSocketNotification("PAGE_INCREMENT", null);
        } else if (rightDist * distDiff <= leftDist) {
          this.sendSocketNotification("PAGE_DECREMENT", null);
        }
      }
    });

    child.stdout.on("exit", (exitCode) => {
      console.log(exitCode);
    });
  }
});
