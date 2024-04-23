#include "LAPSTUDIOSSuegraCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ALAPSTUDIOSSuegraCharacter::ALAPSTUDIOSSuegraCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...    
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character    
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Initialize variables
    bIsMovementInverted = false;
    RandomInversion =0;


}

void ALAPSTUDIOSSuegraCharacter::BeginPlay()
{
    // Call the base class  
    Super::BeginPlay();

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    
    if (Tags.Num() > 0 && Tags[0] == FName(TEXT("Man")))
    {
        RandomInversion = GetRandomFloatBetween10And45();
        FString RandomInversionString = FString::SanitizeFloat(RandomInversion);
        //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("randomValue: %s"), *RandomInversionString));
        InvertMovement1();
    }
    
   
}



void ALAPSTUDIOSSuegraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALAPSTUDIOSSuegraCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALAPSTUDIOSSuegraCharacter::Look);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

void ALAPSTUDIOSSuegraCharacter::Move(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        if (Tags.Num() > 0 && Tags[0] == FName(TEXT("Man"))) 
        {
            float ForwardInput = !bIsMovementInverted ? -MovementVector.Y : MovementVector.Y;
            float RightInput = !bIsMovementInverted ? -MovementVector.X : MovementVector.X;

            // add movement 
            AddMovementInput(ForwardDirection, ForwardInput);
            AddMovementInput(RightDirection, RightInput);
        }
        else if(Tags.Num() > 0 && Tags[0] == FName(TEXT("Woman")))
        {
            // Determine the movement input based on whether inversion is enabled
            float ForwardInput = bIsMovementInverted ? -MovementVector.Y : MovementVector.Y;
            float RightInput = bIsMovementInverted ? -MovementVector.X : MovementVector.X;

            // add movement 
            AddMovementInput(ForwardDirection, ForwardInput);
            AddMovementInput(RightDirection, RightInput);
        }
    }
}

void ALAPSTUDIOSSuegraCharacter::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ALAPSTUDIOSSuegraCharacter::InvertMovement1()
{
    RevertMovementInversion();
    GetWorldTimerManager().SetTimer(MovementRevertTimerHandle, this, &ALAPSTUDIOSSuegraCharacter::InvertMovement2,  RandomInversion, false, RandomInversion);
    
}

void ALAPSTUDIOSSuegraCharacter::InvertMovement2()
{
    RevertMovementInversion();
    GetWorldTimerManager().SetTimer(MovementRevertTimerHandle, this, &ALAPSTUDIOSSuegraCharacter::InvertMovement1,  RandomInversion, false, RandomInversion);
    
}

void ALAPSTUDIOSSuegraCharacter::RevertMovementInversion()
{
    RandomInversion = GetRandomFloatBetween10And45();
    
   // // Convertir el valor de RandomInversion a FString para mostrarlo en pantalla
   //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("randomValue: %f"), RandomInversion));
    
    // Toggle movement inversion
    bIsMovementInverted = !bIsMovementInverted;
     
}

float ALAPSTUDIOSSuegraCharacter::GetRandomFloatBetween10And45()
{
    // Crea un generador de números aleatorios
    FRandomStream RandomStream(FMath::Rand());

    // Genera un número flotante aleatorio entre 10.0 y 45.0
    float RandomFloat = RandomStream.FRandRange(10.0f, 45.0f);

    return RandomFloat;
}

