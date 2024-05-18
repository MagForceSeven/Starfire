
#pragma once

#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"

#include "DataDefinitionThumbnailRenderer.generated.h"

// Implementation of a thumbnail renderer to show icons in the content browser for Data Definitions
UCLASS( )
class STARFIREASSETSEDITOR_API UDataDefinitionThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY( )
public:

	// Thumbnail Renderer API
	bool CanVisualizeAsset( UObject *Object ) override;
	void Draw( UObject *Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget *Viewport, FCanvas *Canvas, bool bAdditionalViewFamily ) override;
};