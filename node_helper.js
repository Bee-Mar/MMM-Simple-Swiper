import { create } from "node_helper";
import { join } from "path";

export default create({
  socketNotificationReceived: (notification, payload) => {
    const threshold = payload["threshold"];
    const distDiff = payload["distanceDiff"];
    const debug_mode = payload["debug"];

    // creating demon spawn
    const child = require("child_process").spawn("sudo", [
      join(__dirname, "/build/mmm_simple_swiper"),
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

      if (debug_mode) {
        console.log("LEFT: " + leftDist + ", RIGHT: " + rightDist);
      }

      // first check if the values are within the threshold distance
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
