(1) Update
・まずは全員を動かしてみて、その結果めり込んでしまった奴らを後から押し戻している。

(2) Integrate
・MyVector3 vel = obj->GetVelocity();
　MyVector3 pos = obj->transform.GetPosition();で現在の速度と座標を取得
・vel.y += GRAVITY * dt;重力を適用する。
・重力とdtを掛け合わせたものを足すことで徐々にに落下速度が速　くなる重力落下を再現。
・pos.x += vel.x * dt;
　pos.y += vel.y * dt;
　pos.z += vel.z * dt;速さと時間で位置を出す。
・obj->SetVelocity(vel);
　obj->transform.SetPosition(pos.x, pos.y, pos.z);
　obj->transform.UpdateMatrix();
　計算で新しくなった速度と座標を適用する。
※ DeltaTimeを引数にしdtを掛けるとどんな性能のPCでも「1秒間に進む距離」を同じに統一している。

(3) ResolveCollisions