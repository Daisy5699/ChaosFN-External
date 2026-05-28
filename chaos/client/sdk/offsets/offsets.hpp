#pragma once

namespace offsets
{
	namespace Globals
	{
		inline constexpr std::int32_t GWorld = 0x1963D8F0;
		inline constexpr std::int32_t GEngine = 0x1963F198;
	}

	namespace World
	{
		inline constexpr std::int32_t PersistentLevel = 0x40; // ULevel*
		inline constexpr std::int32_t Levels = 0x1e0; // TArray<ULevel*>
		inline constexpr std::int32_t GameState = 0x1c8; // AGameStateBase*
		inline constexpr std::int32_t OwningGameInstance = 0x240; // UGameInstance*
	}

	namespace Level
	{
		inline constexpr std::int32_t Actors = 0x38; // TArray<AActor*>
	}

	namespace GameInstance
	{
		inline constexpr std::int32_t LocalPlayers = 0x38; // TArray<ULocalPlayer*>
	}

	namespace Player
	{
		inline constexpr std::int32_t PlayerController = 0x30; // APlayerController*
	}

	namespace LocalPlayer
	{
		inline constexpr std::int32_t ViewportClient = 0x78; // UGameViewportClient*
	}

	namespace PlayerController
	{
		inline constexpr std::int32_t AcknowledgedPawn = 0x358; // APawn*
		inline constexpr std::int32_t PlayerCameraManager = 0x368; // APlayerCameraManager*
	}

	namespace Pawn
	{
		inline constexpr std::int32_t PlayerState = 0x2d0; // APlayerState*
	}

	namespace Actor
	{
		inline constexpr std::int32_t RootComponent = 0x1b0; // USceneComponent*
	}

	namespace Character
	{
		inline constexpr std::int32_t Mesh = 0x330; // USkeletalMeshComponent*
		inline constexpr std::int32_t CharacterMovement = 0x338; // UCharacterMovementComponent*
	}

	namespace GameStateBase
	{
		inline constexpr std::int32_t PlayerArray = 0x2c8; // TArray<APlayerState*>
	}

	namespace PlayerState
	{
		inline constexpr std::int32_t PlayerId = 0x2b4; // int32
		inline constexpr std::int32_t Score = 0x2b0; // float
		inline constexpr std::int32_t Ping = 0x0240; // uint8
		inline constexpr std::int32_t PawnPrivate = 0x328; // APawn*
		inline constexpr std::uint32_t PlayerNamePrivate = 0x348; // FString
	}

	namespace PlayerCameraManager
	{
		inline constexpr std::int32_t CameraCachePrivate = 0x15d0; // FCameraCacheEntry
	}

	namespace CameraCacheEntry
	{
		inline constexpr std::int32_t POV = 0x0010; // FMinimalViewInfo
	}

	namespace MinimalViewInfo
	{
		inline constexpr std::int32_t Location = 0x0000; // FVector
		inline constexpr std::int32_t Rotation = 0x000c; // FRotator
		inline constexpr std::int32_t FOV = 0x0018; // float
	}
}
