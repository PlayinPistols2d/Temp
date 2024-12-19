<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/> 
    <title>Неоновый Слот</title>
    <link rel="stylesheet" href="styles.css" />
</head>
<body>
    <div class="start-screen">
        <h1 class="blinking-title">WELCOME TO NEON CASINO</h1>
        <button class="start-btn">START</button>
    </div>

    <div class="slot-machine" style="display:none;">
        <div class="frame">
            <div class="reel-container">
                <div class="reel" id="reel1"></div>
                <div class="reel" id="reel2"></div>
                <div class="reel" id="reel3"></div>
            </div>
            <button class="spin-btn">SPIN</button>
            <div class="lever-container">
                <div class="lever-handle"></div>
                <div class="lever-knob"></div>
            </div>
        </div>
    </div>
    
    <div class="result-message"></div>
    <div class="emoji-fireworks"></div>
    
    <script src="script.js"></script>
</body>
</html>






body {
    background: #000;
    font-family: 'Arial', sans-serif;
    margin: 0;
    padding: 0;
    overflow: hidden;
    color: #fff;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    height: 100vh;
}

.start-screen {
    text-align: center;
    position: absolute;
    z-index: 100;
}

.start-btn {
    margin-top: 50px;
    padding: 20px 40px;
    font-size: 2rem;
    background: #111;
    color: #0ff;
    border: 4px solid #0ff;
    cursor: pointer;
    border-radius: 10px;
    text-shadow: 0 0 10px #0ff;
    transition: background 0.3s;
}
.start-btn:hover {
    background: #0ff;
    color: #000;
    box-shadow: 0 0 10px #0ff;
}

.blinking-title {
    font-size: 4rem;
    color: #0ff;
    text-shadow: 0 0 20px #0ff;
    animation: neon-blink 1.5s infinite;
}

@keyframes neon-blink {
    0%, 50%, 100% {
        opacity: 1;
    }
    25%, 75% {
        opacity: 0.5;
    }
}

.slot-machine {
    position: relative;
    display: flex;
    justify-content: center;
    align-items: center;
    flex-direction: column;
}

.frame {
    position: relative;
    border: 4px solid #0ff;
    width: 600px;
    height: 300px;
    overflow: hidden;
    box-shadow: 0 0 40px #0ff;
    border-radius: 20px;
    background: radial-gradient(circle, #111, #000);
    display: flex;
    align-items: center;
    justify-content: center;
}

.reel-container {
    display: flex;
    width: 80%;
    height: 80%;
    position: relative;
    z-index: 2;
}

.reel {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: start;
    position: relative;
    overflow: hidden;
    font-size: 3rem;
    color: #0ff;
    text-shadow: 0 0 5px #0ff;
    padding: 10px 0;
}

.reel .symbol {
    height: 80px;
    width: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
    filter: drop-shadow(0 0 5px #0ff);
    text-transform: uppercase;
    font-size: 3rem;
}

/* Рычаг */
.lever-container {
    position: absolute;
    right: -50px;
    top: 50%;
    transform: translateY(-50%);
    width: 40px;
    height: 150px;
    display: flex;
    flex-direction: column;
    align-items: center;
    cursor: pointer;
    z-index: 3;
}

.lever-handle {
    width: 10px;
    height: 100px;
    background: linear-gradient(to bottom, #0ff, #000);
    border: 2px solid #0ff;
    box-shadow: 0 0 10px #0ff;
    border-radius: 5px;
    transition: transform 0.2s ease-out;
    transform-origin: top center;
}

.lever-knob {
    width: 40px;
    height: 40px;
    background: #0ff;
    border: 2px solid #0ff;
    border-radius: 50%;
    box-shadow: 0 0 10px #0ff;
    margin-top: -10px;
    transition: transform 0.2s ease-out;
    transform-origin: center center;
}

.lever-pulled .lever-handle {
    transform: rotate(30deg);
}

.lever-pulled .lever-knob {
    transform: translateY(20px);
}

/* Кнопка SPIN */
.spin-btn {
    position: absolute;
    left: -120px;
    top: 50%;
    transform: translateY(-50%);
    padding: 20px 40px;
    font-size: 2rem;
    background: #111;
    color: #0ff;
    border: 4px solid #0ff;
    cursor: pointer;
    border-radius: 10px;
    text-shadow: 0 0 10px #0ff;
    transition: background 0.3s;
    z-index: 3;
    animation: spin-btn-blink 2s infinite;
}
.spin-btn:hover {
    background: #0ff;
    color: #000;
    box-shadow: 0 0 10px #0ff;
}

@keyframes spin-btn-blink {
    0%, 100% {
        box-shadow: 0 0 10px #0ff;
    }
    50% {
        box-shadow: 0 0 20px #0ff;
    }
}

.result-message {
    position: absolute;
    top: 50px;
    font-size: 3rem;
    text-align: center;
    width: 100%;
    color: #0ff;
    text-shadow: 0 0 20px #0ff;
    opacity: 0;
    transition: opacity 1s;
    pointer-events: none;
    z-index: 999;
}

.emoji-fireworks {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    pointer-events: none;
    overflow: hidden;
}

.emoji {
    position: absolute;
    font-size: 3rem;
    will-change: transform;
    animation: fall 2s ease-out forwards;
}

@keyframes fall {
    0% { 
        opacity: 1; 
        transform: translateY(-100px) scale(0.5) rotate(0deg); 
    }
    100% { 
        opacity: 0; 
        transform: translateY(800px) scale(1) rotate(360deg); 
    }
}







document.addEventListener('DOMContentLoaded', () => {
    const symbols = ['🍒', '🍋', '🍉', '⭐', '💎', '🔔', '7️⃣', '🍀'];
    const symbolHeight = 80;
    const reelSize = 20; // количество символов в одном барабане
    
    const reel1 = document.getElementById('reel1');
    const reel2 = document.getElementById('reel2');
    const reel3 = document.getElementById('reel3');
    const reels = [reel1, reel2, reel3];
    
    const startScreen = document.querySelector('.start-screen');
    const startBtn = document.querySelector('.start-btn');
    const slotMachine = document.querySelector('.slot-machine');
    const resultMessage = document.querySelector('.result-message');
    const emojiContainer = document.querySelector('.emoji-fireworks');

    const leverContainer = document.querySelector('.lever-container');
    const spinBtn = document.querySelector('.spin-btn');

    let isSpinning = false;

    // Для анимации
    let reelPositions = [0,0,0];        // Текущее смещение по Y для каждого барабана
    let reelSpeeds = [0,0,0];           // Скорость вращения барабанов (px/frame)
    let reelRunning = [false, false, false]; // Идет ли анимация для конкретного барабана
    let stopping = [false, false, false];    // Флаги, что барабан в процессе остановки

    let lastTime = 0;

    startBtn.addEventListener('click', () => {
        startScreen.style.display = 'none';
        slotMachine.style.display = 'flex';
        reels.forEach(fillReel);
    });
    
    // Клик по рычагу
    leverContainer.addEventListener('click', () => {
        if (!isSpinning) {
            pullLever();
            startSpinningProcess();
        }
    });
    
    // Клик по кнопке SPIN
    spinBtn.addEventListener('click', () => {
        if (!isSpinning) {
            startSpinningProcess();
        }
    });

    function fillReel(reel) {
        reel.innerHTML = '';
        for (let i = 0; i < reelSize; i++) {
            const symbol = document.createElement('div');
            symbol.classList.add('symbol');
            symbol.textContent = symbols[Math.floor(Math.random() * symbols.length)];
            reel.appendChild(symbol);
        }
    }

    function startSpinningProcess() {
        isSpinning = true;
        resultMessage.style.opacity = 0;

        // Обновим символы
        reels.forEach(fillReel);

        // Инициализируем скорость и состояние
        for (let i = 0; i < 3; i++) {
            reelPositions[i] = 0;
            reelSpeeds[i] = 50; // начальная скорость прокрутки (px/frame)
            reelRunning[i] = true;
            stopping[i] = false;
        }
        
        lastTime = performance.now();
        requestAnimationFrame(animationFrame);

        // Задаем время остановки барабанов
        setTimeout(() => stopReel(0), 2000);
        setTimeout(() => stopReel(1), 3000);
        setTimeout(() => stopReel(2), 4000);
    }

    function animationFrame(time) {
        const delta = time - lastTime;
        lastTime = time;

        // Обновляем каждый барабан
        for (let i = 0; i < 3; i++) {
            if (reelRunning[i]) {
                // Смещаем барабан по скорости
                reelPositions[i] += reelSpeeds[i] * (delta / 16.67); 
                // Ограничим рост, чтобы был бесконечный цикл
                const maxOffset = reelSize * symbolHeight;
                if (reelPositions[i] > maxOffset) {
                    reelPositions[i] = reelPositions[i] % maxOffset;
                }

                // Применяем трансформацию
                reels[i].style.transform = `translateY(-${reelPositions[i]}px)`;

                // Если барабан в процессе остановки, замедляем его
                if (stopping[i] && reelSpeeds[i] > 0) {
                    // Плавное замедление
                    reelSpeeds[i] -= 0.5 * (delta/16.67); // уменьшаем скорость 
                    if (reelSpeeds[i] <= 0) {
                        // Скорость уменьшилась до нуля, выравниваем символ
                        reelSpeeds[i] = 0;
                        finalizeReel(i);
                    }
                }
            }
        }

        // Если хотя бы один барабан еще крутится, продолжаем анимацию
        if (reelRunning.some(r => r === true)) {
            requestAnimationFrame(animationFrame);
        }
    }

    function stopReel(i) {
        stopping[i] = true;
    }

    function finalizeReel(i) {
        // Выравниваем символы
        const remainder = reelPositions[i] % symbolHeight;
        reelPositions[i] -= remainder;
        reels[i].style.transform = `translateY(-${reelPositions[i]}px)`;

        reelRunning[i] = false;
        // Если это последний остановившийся барабан
        if (!reelRunning.some(r => r === true)) {
            // Все остановились, проверяем результат
            checkResult();
            isSpinning = false;
        }
    }

    function checkResult() {
        const finalSymbols = reels.map(reel => reel.children[2].textContent);
        const [s1, s2, s3] = finalSymbols;
        if (s1 === s2 && s2 === s3) {
            showWinAnimation();
        } else {
            showLoseMessage();
        }
    }

    function showWinAnimation() {
        resultMessage.textContent = 'YOU WIN!!!';
        resultMessage.style.opacity = 1;
        for (let i = 0; i < 50; i++) {
            createEmoji();
        }
    }

    function showLoseMessage() {
        resultMessage.textContent = 'YOU LOSE! TRY AGAIN!';
        resultMessage.style.opacity = 1;
    }

    function createEmoji() {
        const emojiList = ['🎉','🎊','💥','✨','🎆','🔥','💎','⭐'];
        const emoji = document.createElement('div');
        emoji.textContent = emojiList[Math.floor(Math.random() * emojiList.length)];
        emoji.classList.add('emoji');
        emoji.style.left = Math.random() * 100 + '%';
        emoji.style.top = '-50px';
        emojiContainer.appendChild(emoji);

        emoji.addEventListener('animationend', () => {
            emoji.remove();
        });
    }

    function pullLever() {
        leverContainer.classList.add('lever-pulled');
        setTimeout(() => {
            leverContainer.classList.remove('lever-pulled');
        }, 500);
    }
});
