<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Neon Drive Game</title>
  <link rel="stylesheet" href="styles.css">
</head>
<body>
  <div id="game-container">
    <div id="car"></div>
    <div id="road"></div>
  </div>
  <div id="score-board">Score: 0</div>
  <script src="game.js"></script>
</body>
</html>



/* styles.css */
body {
  margin: 0;
  overflow: hidden;
  font-family: Arial, sans-serif;
}

#game-container {
  position: relative;
  width: 100vw;
  height: 100vh;
  background: linear-gradient(to top, #ff6b6b, #ffcc00, #3b3b98);
  overflow: hidden;
}

#road {
  position: absolute;
  top: 0;
  width: 100%;
  height: 200%;
  background: repeating-linear-gradient(
    to bottom,
    rgba(255, 255, 255, 0.2),
    rgba(255, 255, 255, 0.2) 20px,
    transparent 20px,
    transparent 40px
  );
  animation: road-scroll 2s linear infinite;
}

@keyframes road-scroll {
  from {
    transform: translateY(0);
  }
  to {
    transform: translateY(-100%);
  }
}

#car {
  position: absolute;
  bottom: 50px;
  left: 50%;
  width: 50px;
  height: 80px;
  background: linear-gradient(to bottom, #faff00, #ff007f);
  border-radius: 10px;
  transform: translateX(-50%);
}

.coin {
  position: absolute;
  width: 20px;
  height: 20px;
  background: radial-gradient(circle, #ffe066, #ffba08);
  border-radius: 50%;
}

.obstacle {
  position: absolute;
  width: 50px;
  height: 50px;
  background: linear-gradient(to bottom, #ff4d4d, #cc0000);
  border-radius: 5px;
}

#score-board {
  position: absolute;
  top: 10px;
  left: 10px;
  color: white;
  font-size: 24px;
  text-shadow: 2px 2px 5px black;
}




// game.js
const gameContainer = document.getElementById('game-container');
const car = document.getElementById('car');
const road = document.getElementById('road');
const scoreBoard = document.getElementById('score-board');

let score = 0;
let carPosition = 50; // Horizontal position (percentage)
let isGameOver = false;

// Add event listeners for car movement
document.addEventListener('keydown', (e) => {
  if (e.key === 'ArrowLeft' && carPosition > 10) carPosition -= 10;
  if (e.key === 'ArrowRight' && carPosition < 90) carPosition += 10;
  car.style.left = `${carPosition}%`;
});

// Create coins and obstacles dynamically
function createElement(className) {
  const element = document.createElement('div');
  element.classList.add(className);
  element.style.left = `${Math.random() * 90}%`;
  element.style.top = '0px';
  gameContainer.appendChild(element);
  return element;
}

// Move coins and obstacles
function moveElement(element, speed, callback) {
  const interval = setInterval(() => {
    const rect = element.getBoundingClientRect();
    element.style.top = `${rect.top + speed}px`;

    // Collision detection
    const carRect = car.getBoundingClientRect();
    if (
      rect.top < carRect.bottom &&
      rect.bottom > carRect.top &&
      rect.left < carRect.right &&
      rect.right > carRect.left
    ) {
      callback();
      clearInterval(interval);
      element.remove();
    }

    // Remove off-screen elements
    if (rect.top > window.innerHeight) {
      clearInterval(interval);
      element.remove();
    }
  }, 20);
}

// Spawn coins and obstacles periodically
function spawn() {
  if (isGameOver) return;

  // Create coins and obstacles
  const coin = createElement('coin');
  moveElement(coin, 3, () => {
    score += 10;
    scoreBoard.textContent = `Score: ${score}`;
  });

  const obstacle = createElement('obstacle');
  moveElement(obstacle, 5, () => {
    isGameOver = true;
    alert(`Game Over! Your score: ${score}`);
    window.location.reload();
  });

  setTimeout(spawn, 1000); // Spawn every second
}

// Start the game
spawn();