#include "DeathParticle.h"  

void DeathParticles::Initialize(Vector3& position, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command) {

   for (auto& particle : worldTransforms_) {  
       particle.model.Initialize(device, descriptor);
       particle.model.Load(command, "deathParticle.obj", "deathParticle.png", 5);
       particle.transform = { {1.0f, 1.0f, 1.0f},{0.0f,0.0f,0.0f},{0.0f, 0.0f, 0.0f} };  
       particle.transform.translate = position;  
   }  

}  

void DeathParticles::Update() {  

   //if (isFinished_) {  
   //    return;  
   //}  

   for (uint32_t i = 0; i < kNumParticles; i++) {  
       // 基本となる速度ベクトル  
       Vector3 velocity = { kSpeed, 0, 0 };  
       // 回転角を計算する  
       float angle = kAngleUnit * i;  
       // Z軸回り回転行列  
       Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);  
       // 基本ベクトルを回転させて速度ベクトルを得る  
       velocity = TransformMatrix(velocity, matrixRotation);  
       // 移動処理  
       worldTransforms_[i].transform.translate += velocity;  
   }  

   counter_ += 1.0f / 60.0f;  
   
   // カウンターを1フレーム分の秒数進める  
   if (counter_ >= kDuraction) {  
       counter_ = kDuraction;  
       // 終了扱いにする  
       isFinished_ = true;  
   }  

}  

void DeathParticles::Draw(Renderer renderer, DebugCamera debugCamera) {  

   // 終了なら何もしない  
   //if (isFinished_) {  
   //    return;  
   //}  

   for (auto& particle : worldTransforms_) {
           particle.model.Draw(renderer, particle.transform, debugCamera.GetViewMatrix());
   }  
}

void DeathParticles::SetPos(Vector3 transform)
{

    for (auto& particle : worldTransforms_) {
        particle.transform.translate = transform;
    }
}
