document.addEventListener('DOMContentLoaded', () => {
    // Замените эти названия на свои PNG-файлы:
    // Например, cherry.png, lemon.png, watermelon.png и т.д.
    const symbols = ['cherry', 'lemon', 'watermelon', 'star', 'diamond', 'bell', 'seven', 'clover'];
    const symbolHeight = 80;
    const reelSize = 40; // Кол-во символов в каждом барабане
    
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
            const symbol = document.createElement('div');
            symbol.classList.add('symbol');
            const icon = symbols[Math.floor(Math.random() * symbols.length)];
            symbol.innerHTML = `<img src="items/${icon}.png" alt="${icon}" />`;
            reel.appendChild(symbol);
        }
        reel.style.transform = 'translateY(0)';
    }

    function startSpin() {
        isSpinning = true;
        resultMessage.style.opacity = 0;

        container.classList.add('spinning');
        reels.forEach(fillReel);

        spinningCount = 3;

        reels.forEach((reel, i) => {
            const duration = 2 + Math.random() * 2; // 2-4 секунды
            const rounds = 5 + Math.floor(Math.random() * 6); 
            const finalOffset = rounds * reelSize * symbolHeight;

            reel.style.transition = `transform ${duration.toFixed(2)}s cubic-bezier(0.33, 0.66, 0.66, 1)`;
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
        // Выравниваем барабан по символу
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
            return symbolImg ? symbolImg.getAttribute('alt') : '';
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
        // Можно убрать имитацию "фейерверков" или заменить на другую анимацию,
        // так как мы отказались от эмодзи.
        // Просто не вызываем createEmoji() если не нужны фейерверки
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