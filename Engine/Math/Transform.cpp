#include "Transform.hpp"

Transform::Transform() {
	SetPRS(Vec3(0.0f), Quaternion::Identity(), Vec3(1.0f));
	Local = Matrix4::Identity();
	Parent = nullptr;
}

Transform::Transform(const Transform& trans) {
	SetPRS(trans.GetPosition(), trans.GetRotation(), trans.GetScale());
	Local = trans.Local;
	Parent = trans.Parent;
}

Transform::Transform(const Vec3& position) {
	SetPRS(position, Quaternion::Identity(), Vec3(1.0f));
	Local = Matrix4::Identity();
	Parent = nullptr;
}

Transform::Transform(const Quaternion& rotation) {
	SetPRS(Vec3(0.0f), rotation, Vec3(1.0f));
	Local = Matrix4::Identity();
	Parent = nullptr;
}

Transform::Transform(const Vec3& position, const Quaternion& rotation) {
	SetPRS(position, rotation, Vec3(1.0f));
	Local = Matrix4::Identity();
	Parent = nullptr;
}

Transform::Transform(const Vec3& position, const Quaternion& rotation, const Vec3& scale) {
	SetPRS(position, rotation, scale);
	Local = Matrix4::Identity();
	Parent = nullptr;
}

void Transform::Translate(const Vec3& translation) {
	vPosition = vPosition + translation;
	IsDirty = true;
}

void Transform::Rotate(const Quaternion& rotation) {
	Quaternion r = rotation;
	vRotation = r.QuaternionMultiply(vRotation);
	IsDirty = true;
}

void Transform::Scale(const Vec3& scale) {
	vScale = vScale * scale;
	IsDirty = true;
}

void Transform::SetPR(const Vec3& pos, const Quaternion& rotation) {
	vPosition = pos;
	vRotation = rotation;
	IsDirty = true;
}

void Transform::SetPRS(const Vec3& pos, const Quaternion& rotation, const Vec3& scale) {
	vPosition = pos;
	vRotation = rotation;
	vScale = scale;
	IsDirty = true;
}

void Transform::TransformRotate(const Vec3& translation, const Quaternion& rotation) {
	vPosition = vPosition + translation;
	Quaternion r = rotation;
	vRotation = r.QuaternionMultiply(vRotation);
	IsDirty = true;
}

Matrix4 Transform::GetLocal() {
	if (IsDirty) {
		UpdateLocal();
	}

	return Local;
}

void Transform::UpdateLocal() {
	Matrix4 R = QuatToMatrix(vRotation);
	Matrix4 T = Matrix4::FromTranslation(vPosition);
	Matrix4 S = Matrix4::FromScale(vScale);

	Matrix4 TempLocal = S.Multiply(R);
	Local = TempLocal.Multiply(T);
	IsDirty = false;
}

Matrix4 Transform::GetWorldTransform() {
	Matrix4 l = GetLocal();
	if (Parent != nullptr) {
		Matrix4 p = Parent->GetWorldTransform();
		return l.Multiply(p);
	}

	return l;
}