# include <Siv3D.hpp>

// 難易度の定義
enum class Difficulty { Easy, Normal, Hard };
enum class State { Title, Game, Result };

struct SumoSans {
	P2Body body;
	double radius = 40.0;
	ColorF color;
};

void Main() {
	Window::Resize(800, 600);
	const Font font{ 30, Typeface::Medium };
	const Font titleFont{ 80, Typeface::Heavy };

	P2World world{ 0.0 };
	const Circle dohyo{ Scene::Center(), 220 };

	State state = State::Title;
	Difficulty difficulty = Difficulty::Normal; // デフォルト難易度
	int32 day = 1, wins = 0, loses = 0;
	bool isWin = false;

	Optional<SumoSans> player;
	Optional<SumoSans> enemy;

	// --- 取組の初期化関数 ---
	auto SetupMatch = [&]() {
		world.update();
		
		// 自力士
		player = SumoSans{ world.createCircle(P2Dynamic, Scene::Center().movedBy(-120, 0), 40), 40, Palette::Royalblue };
		player->body.setFixedRotation(true);
		player->body.setDamping(3.0); // プレイヤーも少し粘りを持たせる
		
		// 敵力士
		enemy = SumoSans{ world.createCircle(P2Dynamic, Scene::Center().movedBy(120, 0), 40), 40, Palette::Crimson };
		enemy->body.setFixedRotation(true);

		// 難易度によるパラメータ分岐
		if (difficulty == Difficulty::Easy) {
			enemy->body.setDamping(8.0);  // 非常に重い（動きが鈍い）
			//enemy->body.setMass(0.8);     // 軽い（押し出しやすい）
		}
		else if (difficulty == Difficulty::Normal) {
			enemy->body.setDamping(5.0);  // 標準
			//enemy->body.setMass(1.0);     // 標準
		}
		else if (difficulty == Difficulty::Hard) {
			enemy->body.setDamping(2.0);  // 軽快（キビキビ動く）
			//enemy->body.setMass(1.5);     // 重い（押し出しにくい）
		}
	};

	while (System::Update()) {
		switch (state) {
		case State::Title:
			Scene::SetBackground(Palette::Darkslateblue);
			titleFont(U"Siv3D大相撲").drawAt(Scene::Center().movedBy(0, -150));
			
			// 難易度選択ボタン
			if (SimpleGUI::Button(U"初級 (Easy)", Scene::Center().movedBy(-100, -20), 200, difficulty != Difficulty::Easy)) difficulty = Difficulty::Easy;
			if (SimpleGUI::Button(U"中級 (Normal)", Scene::Center().movedBy(-100, 30), 200, difficulty != Difficulty::Normal)) difficulty = Difficulty::Normal;
			if (SimpleGUI::Button(U"上級 (Hard)", Scene::Center().movedBy(-100, 80), 200, difficulty != Difficulty::Hard)) difficulty = Difficulty::Hard;

			if (SimpleGUI::Button(U"場所を始める", Scene::Center().movedBy(-100, 160), 200, Palette::Orange)) {
				day = 1; wins = 0; loses = 0;
				SetupMatch();
				state = State::Game;
			}
			break;

		case State::Game:
			Scene::SetBackground(ColorF{ 0.2, 0.15, 0.1 });
			world.update();

			dohyo.draw(Palette::Antiquewhite).drawFrame(10, Palette::Sienna);
			font(U"{}日目 ({}勝 {}敗) - 難易度: {}"_fmt(day, wins, loses, 
				difficulty == Difficulty::Easy ? U"初級" : (difficulty == Difficulty::Normal ? U"中級" : U"上級")))
				.draw(20, 20);

			if (player && enemy) {
				// プレイヤー操作
				Vec2 force{ 0, 0 };
				if (KeyLeft.pressed())  force.x -= 500;
				if (KeyRight.pressed()) force.x += 500;
				if (KeyUp.pressed())    force.y -= 500;
				if (KeyDown.pressed())  force.y += 500;
				player->body.applyForce(force * 200);

				// 敵AI: 難易度によって押し出す力を変える
				double aiPower = 15000;
				if (difficulty == Difficulty::Easy)   aiPower = 8000;
				if (difficulty == Difficulty::Hard)   aiPower = 25000;

				Vec2 toPlayer = (player->body.getPos() - enemy->body.getPos()).setLength(aiPower);
				enemy->body.applyForce(toPlayer);

				// 描画
				player->body.draw(player->color);
				enemy->body.draw(enemy->color);

				if (!dohyo.intersects(player->body.getPos())) {
					loses++; isWin = false; state = State::Result;
				}
				else if (!dohyo.intersects(enemy->body.getPos())) {
					wins++; isWin = true; state = State::Result;
				}
			}
			break;

		case State::Result:
			dohyo.draw(Palette::Antiquewhite).drawFrame(10, Palette::Sienna);
			if (player) player->body.draw(player->color);
			if (enemy) enemy->body.draw(enemy->color);

			titleFont(isWin ? U"勝利！" : U"敗北...").drawAt(Scene::Center().movedBy(0, -100), isWin ? Palette::Gold : Palette::Black);

			if (SimpleGUI::Button(U"次へ", Scene::Center().movedBy(-100, 100), 200)) {
				if (wins >= 8 || loses >= 8 || day >= 15) state = State::Title;
				else { day++; SetupMatch(); state = State::Game; }
			}
			break;
		}
	}
}
