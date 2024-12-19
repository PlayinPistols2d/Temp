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
    position: relative;
    overflow: hidden;
    font-size: 3rem;
    color: #0ff;
    text-shadow: 0 0 5px #0ff;
    padding: 10px 0;
    transform: translateY(0);
    will-change: transform;
    backface-visibility: hidden;
}

.symbol {
    height: 80px;
    width: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
    background: #222; /* фоновый цвет для визуализации */
    position: relative;
    overflow: hidden;
}

.symbol img {
    max-width: 70px; /* чуть меньше чем 80px, чтобы был отступ */
    max-height: 70px;
    display: block;
    object-fit: contain;
    transform: translateZ(0);
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
    const symbols = ['cherry', 'lemon', 'watermelon', 'star', 'diamond', 'bell', 'seven', 'clover'];
    const symbolHeight = 80;
    const reelSize = 40;
    
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
    const container = document.getElementById('container');

    let isSpinning = false;
    let spinningCount = 0; 

    startBtn.addEventListener('click', () => {
        startScreen.style.display = 'none';
        slotMachine.style.display = 'flex';
        reels.forEach(fillReel);
    });
    
    leverContainer.addEventListener('click', () => {
        if (!isSpinning) {
            pullLever();
            startSpin();
        }
    });
    
    spinBtn.addEventListener('click', () => {
        if (!isSpinning) {
            startSpin();
        }
    });

    function fillReel(reel) {
        reel.innerHTML = '';
        for (let i = 0; i < reelSize; i++) {
            const symbolDiv = document.createElement('div');
            symbolDiv.classList.add('symbol');
            const icon = symbols[Math.floor(Math.random() * symbols.length)];
            const img = document.createElement('img');
            img.src = `items/${icon}.png`;
            img.alt = icon;
            symbolDiv.appendChild(img);
            reel.appendChild(symbolDiv);
        }
        reel.style.transform = 'translateY(0)';
        // Попытка принудить браузер к отрисовке
        reel.offsetHeight; 
    }

    function startSpin() {
        isSpinning = true;
        resultMessage.style.opacity = 0;

        container.classList.add('spinning');
        reels.forEach(fillReel);

        spinningCount = 3;

        reels.forEach((reel, i) => {
            const duration = 2 + Math.random() * 2; 
            const rounds = 5 + Math.floor(Math.random() * 6); 
            const finalOffset = rounds * reelSize * symbolHeight;

            reel.style.transition = `transform ${duration.toFixed(2)}s ease-out`;
            reel.offsetHeight; // Force reflow
            reel.style.transform = `translateY(-${finalOffset}px)`;

            reel.addEventListener('transitionend', function onTransitionEnd() {
                reel.removeEventListener('transitionend', onTransitionEnd);
                finalizeReel(i, finalOffset);
            });
        });
    }

    function finalizeReel(index, finalOffset) {
        const offsetSymbols = (finalOffset / symbolHeight);
        const centerIndex = (offsetSymbols + 2) % reelSize;

        const reel = reels[index];
        reel.style.transition = 'none';
        const adjustOffset = Math.floor(centerIndex) * symbolHeight;
        reel.style.transform = `translateY(-${adjustOffset}px)`;

        spinningCount--;
        if (spinningCount === 0) {
            container.classList.remove('spinning');
            checkResult();
            isSpinning = false;
        }
    }

    function checkResult() {
        const finalSymbols = reels.map(reel => {
            const transformValue = reel.style.transform;
            const match = transformValue.match(/-([\d]+)px\)/);
            let offset = match ? parseInt(match[1]) : 0;
            let topSymbolIndex = Math.floor(offset / symbolHeight);
            const centerSymbolIndex = (topSymbolIndex + 2) % reelSize;
            const symbolImg = reel.children[centerSymbolIndex].querySelector('img');
            return symbolImg ? symbolImg.alt : '';
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
    }

    function showLoseMessage() {
        resultMessage.textContent = 'YOU LOSE! TRY AGAIN!';
        resultMessage.style.opacity = 1;
    }

    function pullLever() {
        leverContainer.classList.add('lever-pulled');
        setTimeout(() => {
            leverContainer.classList.remove('lever-pulled');
        }, 500);
    }
});