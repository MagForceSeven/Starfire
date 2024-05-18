
#include "DataDefinitionThumbnailRenderer.h"

#include "DataDefinitions/DataDefinition.h"

// Engine
#include "CanvasItem.h"
#include "Engine/Texture.h"

bool UDataDefinitionThumbnailRenderer::CanVisualizeAsset( UObject *Object )
{
	const auto Definition = Cast< UDataDefinition >( Object );
	if (Definition == nullptr)
		return false;

	const auto Thumbnail = Definition->GetThumbnail( );

	return Thumbnail != nullptr;
}

void UDataDefinitionThumbnailRenderer::Draw( UObject *Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget *Viewport, FCanvas *Canvas, bool bAdditionalViewFamily )
{
	const auto Definition = Cast< UDataDefinition >( Object );
	if (Definition == nullptr)
		return;

	const auto Thumbnail = Definition->GetThumbnail( );
	if (Thumbnail == nullptr)
		return;

	FCanvasTileItem CanvasTile( FVector2D( X, Y ), Thumbnail->GetResource( ), FVector2D( Width, Height ), FLinearColor( 1.0f, 1.0f, 1.0f ) );
	CanvasTile.BlendMode = SE_BLEND_Translucent;
	CanvasTile.Draw( Canvas );
}