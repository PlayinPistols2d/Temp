<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/> 
    <title>Неоновый Слот</title>
    <link rel="stylesheet" href="styles.css" />
</head>
<body>
    <div class="slot-machine">
        <div class="frame">
            <div class="reel-container">
                <div class="reel" id="reel1">
                    <!-- Символы будут добавлены через JS -->
                </div>
                <div class="reel" id="reel2">
                    <!-- Символы будут добавлены через JS -->
                </div>
                <div class="reel" id="reel3">
                    <!-- Символы будут добавлены через JS -->
                </div>
            </div>
            <button class="spin-btn">SPIN</button>
        </div>
    </div>
    <script src="script.js"></script>
</body>
</html>





body {
    background: #000;
    font-family: 'Arial', sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
    color: #fff;
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
    border: 2px solid #0ff;
    width: 400px;
    height: 200px;
    overflow: hidden;
    box-shadow: 0 0 20px #0ff;
    border-radius: 10px;
    background: radial-gradient(circle, #111, #000);
}

.reel-container {
    display: flex;
    width: 100%;
    height: 100%;
    position: relative;
}

.reel {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: start;
    position: relative;
    overflow: hidden;
    font-size: 2rem;
    color: #0ff;
    text-shadow: 0 0 5px #0ff;
    padding: 10px 0;
}

.reel .symbol {
    height: 50px;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: transform 0.5s;
    filter: drop-shadow(0 0 5px #0ff);
    text-transform: uppercase;
}

.spin-btn {
    margin-top: 20px;
    padding: 10px 20px;
    font-size: 1.2rem;
    background: #111;
    color: #0ff;
    border: 2px solid #0ff;
    cursor: pointer;
    border-radius: 5px;
    text-shadow: 0 0 5px #0ff;
    transition: background 0.3s;
}

.spin-btn:hover {
    background: #0ff;
    color: #000;
    box-shadow: 0 0 10px #0ff;
}






document.addEventListener('DOMContentLoaded', () => {
    const symbols = ['🍒', '🍋', '🍉', '⭐', '💎', '🔔', '7️⃣', '🍀'];
    
    const reel1 = document.getElementById('reel1');
    const reel2 = document.getElementById('reel2');
    const reel3 = document.getElementById('reel3');
    
    const reels = [reel1, reel2, reel3];
    
    // Функция для заполнения барабанов символами
    function fillReel(reel) {
        reel.innerHTML = '';
        // Добавим чуть больше символов, чтобы была анимация прокрутки
        for (let i = 0; i < 20; i++) {
            const symbol = document.createElement('div');
            symbol.classList.add('symbol');
            symbol.textContent = symbols[Math.floor(Math.random() * symbols.length)];
            reel.appendChild(symbol);
        }
    }
    
    reels.forEach(fillReel);
    
    const spinBtn = document.querySelector('.spin-btn');

    let isSpinning = false;
    let spinIntervals = [];

    spinBtn.addEventListener('click', () => {
        if (isSpinning) return;
        isSpinning = true;
        
        // Перезаполним барабаны новыми символами перед стартом
        reels.forEach(fillReel);
        
        // Запускаем вращение
        startSpin();
        
        // Останавливаем через некоторое время с небольшими задержками между барабанами
        setTimeout(() => stopReel(0), 2000);
        setTimeout(() => stopReel(1), 3000);
        setTimeout(() => stopReel(2), 4000);
    });
    
    // Функция запуска вращения барабанов
    function startSpin() {
        reels.forEach((reel, idx) => {
            const speed = 50; // скорость переключения символов (мс)
            let offset = 0;
            // Сохраняем интервал для каждого барабана
            spinIntervals[idx] = setInterval(() => {
                offset++;
                reel.style.transform = `translateY(-${offset * 50}px)`;
                // Если достигли конца списка символов, заполняем заново
                if (offset >= reel.children.length - 3) {
                    offset = 0;
                    fillReel(reel);
                    reel.style.transform = `translateY(0px)`;
                }
            }, speed);
        });
    }

    // Функция для остановки конкретного барабана
    function stopReel(reelIndex) {
        clearInterval(spinIntervals[reelIndex]);
        const reel = reels[reelIndex];
        
        // Получаем текущий смещение
        const transformValue = reel.style.transform;
        const match = transformValue.match(/translateY\(-?(\d+)px\)/);
        let offset = match ? parseInt(match[1]) : 0;
        
        // Рассчитываем, до какого символа докрутить, чтобы остановиться ровно
        // Каждый символ 50px, округлим offset до ближайшего символа:
        const symbolHeight = 50;
        const remainder = offset % symbolHeight;
        if (remainder !== 0) {
            offset -= remainder;
        }
        
        // Добавим анимацию замедления при остановке
        reel.style.transition = 'transform 0.5s ease-out';
        reel.style.transform = `translateY(-${offset}px)`;
        
        // После окончания перехода удаляем transition, чтобы при следующем спине не было проблем
        reel.addEventListener('transitionend', function cleanup() {
            reel.style.transition = '';
            reel.removeEventListener('transitionend', cleanup);
            
            // Если это последний барабан, возвращаем флаг
            if (reelIndex === 2) {
                isSpinning = false;
            }
        });
    }
});
