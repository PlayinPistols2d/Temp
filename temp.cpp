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
    const reelVisibleCount = 3;
    const reelSize = 60; // большое количество символов для плавной прокрутки
    
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

    // Параметры анимации
    let reelOffsets = [0,0,0]; // текущее смещение по Y для каждого барабана
    let reelSpeeds = [0,0,0];  // скорость (px за кадр) для каждого барабана
    let reelRunning = [false,false,false];
    let reelStopping = [false,false,false];

    let animationFrameId = null;

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
        // Заполним барабан большим количеством символов
        // Повторим рандомный набор несколько раз
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
        reels.forEach(fillReel);

        // Инициализируем начальные скорости и состояния
        for (let i = 0; i < 3; i++) {
            reelOffsets[i] = 0;
            reelSpeeds[i] = 2.5; // не слишком быстро: ~2.5px/кадр
            reelRunning[i] = true;
            reelStopping[i] = false;
        }

        startAnimation();
        
        // Запускаем таймеры остановки
        setTimeout(() => { reelStopping[0] = true; }, 2000);
        setTimeout(() => { reelStopping[1] = true; }, 3000);
        setTimeout(() => { reelStopping[2] = true; }, 4000);
    }

    function startAnimation() {
        if (animationFrameId) cancelAnimationFrame(animationFrameId);
        animationFrame();
    }

    function animationFrame() {
        let stillRunning = false;

        for (let i = 0; i < 3; i++) {
            if (reelRunning[i]) {
                stillRunning = true;
                // Движение барабана
                reelOffsets[i] += reelSpeeds[i];

                // Если смещение вышло за пределы длины всех символов
                const totalHeight = reelSize * symbolHeight;
                if (reelOffsets[i] > totalHeight) {
                    reelOffsets[i] -= totalHeight; 
                }

                // Применяем смещение
                reels[i].style.transform = `translateY(-${reelOffsets[i]}px)`;

                // Если барабан должен останавливаться, замедляем его
                if (reelStopping[i]) {
                    if (reelSpeeds[i] > 0) {
                        reelSpeeds[i] -= 0.05; // плавное замедление
                        if (reelSpeeds[i] <= 0) {
                            // Скорость упала до нуля, выравниваем
                            reelSpeeds[i] = 0;
                            alignReel(i);
                            reelRunning[i] = false;
                        }
                    }
                }
            }
        }

        if (stillRunning) {
            animationFrameId = requestAnimationFrame(animationFrame);
        } else {
            // Все остановились
            checkResult();
            isSpinning = false;
        }
    }

    function alignReel(i) {
        // Выравниваем барабан по символу
        const remainder = reelOffsets[i] % symbolHeight;
        reelOffsets[i] -= remainder;
        reels[i].style.transform = `translateY(-${reelOffsets[i]}px)`;
    }

    function checkResult() {
        // Центральный символ будет примерно на индексе 2 от начала (0,1,2)
        // Поскольку у нас большое количество символов, мы берем reelOffsets[i], чтобы понять какой сейчас символ в центре.
        // Центрируемся на символе, который соответствует reelOffsets[i].
        // reelOffsets[i] показывает, на сколько px прокручен барабан.
        // Индекс центрального символа: (reelOffsets[i]/symbolHeight) + 2 (берём 2 как позицию "выигрышной линии")
        
        function getCenterSymbol(reel) {
            // Центральную линию возьмём символ с индексом 2 относительно начала видимой области.
            // reelOffsets[i]/symbolHeight даёт сдвиг символов.
            // Предположим, верхний символ - индекс floor(reelOffsets[i]/symbolHeight),
            // а нам нужен символ на линии 2, значит индекс символа:
            // centerIndex = (Math.floor(reelOffsets[i]/symbolHeight) + 2) % reelSize
            return (Math.floor(reelOffsets[reels.indexOf(reel)]/symbolHeight) + 2) % reelSize;
        }

        const finalSymbols = reels.map(reel => {
            const idx = getCenterSymbol(reel);
            return reel.children[idx].textContent;
        });

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