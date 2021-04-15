#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <memory>

#ifdef _DEBUG
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#else
#define DBG_NEW
#endif

#include "cRenderer.hpp"

void cRenderer::RenderScene(std::vector<cModelObject*> scene, cFBO* pFBO_Out, cCamera* camera, GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen)
{
	// 1. Draw everything to proper frame buffer
	if (pFBO_Out == NULL)
	{
		// Rendered to our screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO Change it somehow
		// Is it is a Full-Screen Effect (if there is a SecondPass object)
		GLint bIsFullScreenEffect_UL = glGetUniformLocation(shaderProgID, "bIsFullScreenEffect");
		glUniform1f(bIsFullScreenEffect_UL, (float)GL_TRUE);
	}
	else
	{
		// glIsFramebuffer doesn't work properly! (>__<)
		// Rendered to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, pFBO_Out->ID);
	}

	// 2. Clear the ACTUAL screen buffer (unless otherwise stated)
	if (clearScreen)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// 3. Change the ViewMatrix depending on the camera
	GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
	glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	// 4. Draw objects for the scene
	for (int index = 0; index < scene.size(); index++)
	{
		cModelObject* pCurrentObject = scene[index];
		if (pCurrentObject->isVisible) {
			if (pCurrentObject->objectType == "Skybox")
			{
				pCurrentObject->positionXYZ = camera->GetPosition();
				pCurrentObject->recalculateWorldMatrix();
				RenderSkyBoxObject(pCurrentObject, shaderProgID, pVAOManager);
			}
			else if (pCurrentObject->objectType == "Basic")
			{
				pCurrentObject->recalculateWorldMatrix();
				Render3DObject(pCurrentObject, shaderProgID, pVAOManager);
			}
			else if (pCurrentObject->objectType == "SecondPass")
			{
				pCurrentObject->recalculateWorldMatrix();
				RenderSecondPassObject(pCurrentObject, shaderProgID, pVAOManager);
			}
			else if (pCurrentObject->objectType == "Reflective")
			{
				pCurrentObject->recalculateWorldMatrix();
				RenderReflectiveObject(pCurrentObject, shaderProgID, pVAOManager);
			}
			else if (pCurrentObject->objectType == "Transparent")
			{
				pCurrentObject->recalculateWorldMatrix();
				Render3DObject(pCurrentObject, shaderProgID, pVAOManager);
			}
			//else if (pCurrentObject->friendlyName == "UpperWater")
			//{
			//	waterOffset.x += 0.1f * deltaTime;
			//	waterOffset.y += 0.017f * deltaTime;
			//	pCurrentObject->recalculateWorldMatrix();
			//	pRenderer->Render3DObjectHeightMap(pCurrentObject, shaderProgID, pTheVAOManager, glm::vec2(waterOffset.x, waterOffset.y));
			//}
			//else if (pCurrentObject->friendlyName == "LowerWater")
			//{
			//	waterOffset.z -= 0.13f * deltaTime;
			//	waterOffset.w -= 0.013f * deltaTime;
			//	pCurrentObject->recalculateWorldMatrix();
			//	pRenderer->Render3DObjectMovingTexture(pCurrentObject, shaderProgID, pTheVAOManager, glm::vec2(waterOffset.z, waterOffset.w));
			//}
			//else if (pCurrentObject->friendlyName == "ParticlesCube")
			//{
			//	pMyPartcles->location = glm::vec3(pCurrentObject->positionXYZ.x, pCurrentObject->positionXYZ.y, pCurrentObject->positionXYZ.z);
			//	pMyPartcles->m_minVelocity = glm::vec3(0.0f, 0.0f, 0.0f) * -mainCamera.GetFront();
			//	pMyPartcles->m_maxVelocity = glm::vec3(0.0f, 0.0f, 0.0f) * -mainCamera.GetFront();

			//	glBindBuffer(GL_ARRAY_BUFFER, particles_mat_model_buffer);
			//	glBufferData(GL_ARRAY_BUFFER, pMyPartcles->getMaxParticles() * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
			//	glBufferSubData(GL_ARRAY_BUFFER, 0, pMyPartcles->getParticlesCount() * sizeof(glm::mat4), pMyPartcles->g_particule_mat_model_data);

			//	pCurrentObject->recalculateWorldMatrix();
			//	pRenderer->RenderInstancedParticleObject(pCurrentObject, shaderProgID, pTheVAOManager, pMyPartcles);
			//}
		}
	}

	if (pFBO_Out == NULL)
	{
		GLint bIsFullScreenEffect_UL = glGetUniformLocation(shaderProgID, "bIsFullScreenEffect");
		glUniform1f(bIsFullScreenEffect_UL, (float)GL_FALSE);
	}
}

void cRenderer::RenderSceneWithStencilPortals(std::vector<cModelObject*> mainScene, std::map<std::string, std::vector<cModelObject*>> scenesToPortals,
												cFBO* pFBO_Out, cCamera* camera, GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen)
{
	// 1. Create the vector of portals from given mainScene and check if it's empty
	std::vector<cModelObject*> portals;

	for (int index = 0; index < mainScene.size(); index++)
	{
		cModelObject* pCurrentObject = mainScene[index];
		if (pCurrentObject->objectType == "FlatStencilPortal" || pCurrentObject->objectType == "VolumetricStencilPortal")
		{
			portals.push_back(pCurrentObject);
		}
	}

	if (portals.empty())
	{
		//RenderScene(mainScene, pFBO_Out, camera, shaderProgID, pVAOManager, clearScreen);

		// TODO write it not in console (in logs or smth)
		std::cout << "There is no portals in given Main Scene. Main Scene isn't drawn. Try using RenderScene function instead." << std::endl;

		return;
	}

	// 2. Check if all portals recieved the scenes to point to
	for (int index = 0; index < portals.size(); index++)
	{
		if (scenesToPortals.find(portals[index]->friendlyName) == scenesToPortals.end())
		{
			// TODO write it not in console (in logs or smth)
			std::cout << "There is no scene for the portal to point to. Main Scene isn't drawn. Error." << std::endl;

			return;
		}
	}

	// 3. Draw everything to proper frame buffer
	if (pFBO_Out == NULL)
	{
		// Rendered to our screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO Change it somehow
		// Is it is a Full-Screen Effect (if there is a SecondPass object)
		GLint bIsFullScreenEffect_UL = glGetUniformLocation(shaderProgID, "bIsFullScreenEffect");
		glUniform1f(bIsFullScreenEffect_UL, (float)GL_TRUE);
	}
	else
	{
		// Rendered to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, pFBO_Out->ID);
	}

	// 4. Clear the ACTUAL screen buffer (unless otherwise stated)
	if (clearScreen)
	{
		// glIsFramebuffer doesn't work properly! (>__<)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// 5. Change the ViewMatrix depending on the camera
	GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
	glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));


	// 6. Disable STENCIL TEST before drawing the main scene (Just to be sure)
	glDisable(GL_STENCIL_TEST);

	// 7. Render Main Scene
	RenderScene(mainScene, pFBO_Out, camera, shaderProgID, pVAOManager, clearScreen);

	// 8. For loop for portals and scenes they point to
	for (int portal_index = 0; portal_index < portals.size(); portal_index++)
	{
		cModelObject* pCurrentPortal = portals[portal_index];
		std::vector<cModelObject*> pCurrentScene = scenesToPortals[portals[portal_index]->friendlyName];

		// Render portal and scene it points to
		// Enables and Disables STENCIL TEST inside
		RenderStencilPortalToScene(pCurrentPortal, pCurrentScene, pFBO_Out, camera, shaderProgID, pVAOManager, false);
	}

}

void cRenderer::RenderStencilPortalToScene(cModelObject* portal, std::vector<cModelObject*> scene,
									cFBO* pFBO_Out, cCamera* camera, GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen)
{
	// 1. Draw everything to proper frame buffer
	if (pFBO_Out == NULL)
	{
		// Rendered to our screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO Change it somehow
		// Is it is a Full-Screen Effect (if there is a SecondPass object)
		GLint bIsFullScreenEffect_UL = glGetUniformLocation(shaderProgID, "bIsFullScreenEffect");
		glUniform1f(bIsFullScreenEffect_UL, (float)GL_TRUE);
	}
	else
	{
		// Rendered to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, pFBO_Out->ID);
	}

	// 2. Clear the ACTUAL screen buffer (unless otherwise stated)
	if (clearScreen)
	{
		// glIsFramebuffer doesn't work properly! (>__<)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// 3. Change the ViewMatrix depending on the camera
	GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
	glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	// 4. Enable Stensil Test to render portal and the scene it points to
	glEnable(GL_STENCIL_TEST);

	// 5. Prepare for portal rendering
	// Depending on the type of portal enable/disable Depth Buffer Test and Depth Mask
	if (portal->objectType == "FlatStencilPortal")
	{
		portal->disableDepthBufferTest = true;
		portal->disableDepthBufferWrite = true; //Mask
	}

	if (portal->objectType == "VolumetricStencilPortal")
	{
		portal->disableDepthBufferTest = false;
		portal->disableDepthBufferWrite = false; //Mask
	}

	// Disable Color Mask (You shouldn't see the portal itself)
	portal->disableColorBufferWrite = false; //Mask //Add

	// Declare that Stencil Buffer will be cleared to the number 47
	glClearStencil(47);

	// Clear stencil (to the number 47)
	glClear(GL_STENCIL_BUFFER_BIT);

	glStencilOp(GL_KEEP,		// If Stencil Test fails KEEP the original value (47)
				GL_KEEP,		// If Stencil Test passes and Depth Test fails KEEP the original value (47)
				GL_REPLACE);	// If Stencil Test and Depth Test Both Passes REPLACE with the value from glStencilFunc

	glStencilFunc(GL_ALWAYS,	// Draw ALWAYS (But portal won't be visible bacause Color Mask is Disabled)
					133,		// Used value (Original value is being REPLACED with this value)
					0xFF);		// Mask for the value (1111,1111 or 0xFF means there is no mask)

	// 6. Render Portal
	// Depth Test and Depth Mask are enabled/disabled here
	// Color Mask is disabled here
	portal->recalculateWorldMatrix();
	RenderStencilPortalObject(portal, shaderProgID, pVAOManager);

	// 7. Prepare for scene rendering
	glStencilOp(GL_KEEP,		// If Stencil Test fails KEEP the original value (47 or 133)
				GL_KEEP,		// If Stencil Test passes and Depth Test fails KEEP the original value (47 or 133)
				GL_KEEP);		// If Stencil Test and Depth Test Both Passes Keep the original value (47 or 133)

	glStencilFunc(GL_EQUAL,		// Draw when original value is equal to used value (Color Mask should be enabled)
					133,		// Used value (Used here)
					0xFF);		// Mask for the value (1111,1111 or 0xFF means there is no mask)

	// 8. Enable/Disable Depth Test and Depth Mask for objects depending on the portal type and remember the changes
	std::vector<cModelObject*> changedObjectsDepthTest;
	std::vector<cModelObject*> changedObjectsDepthMask;
	if (portal->objectType == "FlatStencilPortal")
	{
		for (int scene_index = 0; scene_index < scene.size(); scene_index++)
		{
			if (scene[scene_index]->disableDepthBufferTest != true)
			{
				changedObjectsDepthTest.push_back(scene[scene_index]);
				scene[scene_index]->disableDepthBufferTest = true;
			}

			//if (scene[scene_index]->disableDepthBufferWrite != true)
			//{
			//	changedObjectsDepthMask.push_back(scene[scene_index]);
			//	scene[scene_index]->disableDepthBufferWrite = true;
			//}
		}
	}

	// 9. Render the scene portal points to
	// Depth Test, Depth Mask and Color Mask are enabled/disabled here (depending on the object)
	RenderScene(scene, pFBO_Out, camera, shaderProgID, pVAOManager, false);

	// 10. Change scene objects back if needed
	if (!changedObjectsDepthTest.empty())
	{
		for (int index = 0; index < changedObjectsDepthTest.size(); index++)
		{
			changedObjectsDepthTest[index]->disableDepthBufferTest = false;
		}
	}

	if (!changedObjectsDepthMask.empty())
	{
		for (int index = 0; index < changedObjectsDepthMask.size(); index++)
		{
			changedObjectsDepthMask[index]->disableDepthBufferWrite = false;
		}
	}

	// 11. Disable STENCIL TEST after you rendered portal and the scene it points to
	glDisable(GL_STENCIL_TEST);
}

void cRenderer::RenderStencilPortalObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}

	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON (Should always be off)
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	return;
}

void cRenderer::Render3DObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}

	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	return;
}

void cRenderer::RenderSecondPassObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager)
{
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	/*********************************************************************************/

	GLint passNumber_UniLoc = glGetUniformLocation(shaderProgID, "passNumber");
	glUniform1i(passNumber_UniLoc, 1);

	// Set up the texture from FBO
	glActiveTexture(GL_TEXTURE0 + 40);				// Texture Unit 40
	glBindTexture(GL_TEXTURE_2D, FBO_map[pCurrentObject->friendlyName]->colourTexture_0_ID);	// Texture now assoc with the proper texture unit
	//glBindTexture(GL_TEXTURE_2D, FBO_vec[0]->depthTexture_ID);	// Texture now assoc with the proper texture unit
	GLint secondPass_UL = glGetUniformLocation(shaderProgID, "secondPassColourTexture");
	glUniform1i(secondPass_UL, 40);	// Texture unit 40

	if (pCurrentObject->baseEffectUse == true)
	{
		GLint bUseBaseEffect_UL = glGetUniformLocation(shaderProgID, "bUseBaseEffect");
		glUniform1f(bUseBaseEffect_UL, (float)GL_TRUE);
	}

	if (pCurrentObject->overlayEffectTexture != "")
	{
		GLint bUseOverlayEffect_UL = glGetUniformLocation(shaderProgID, "bUseOverlayEffect");
		glUniform1f(bUseOverlayEffect_UL, (float)GL_TRUE);

		// Set up the texture for effect
		GLuint texSamp_ID = mpTexture[pCurrentObject->overlayEffectTexture];
		glActiveTexture(GL_TEXTURE0 + 41);				// Texture Unit 41
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0
		GLint texture_UL = glGetUniformLocation(shaderProgID, "effectTexture");
		glUniform1i(texture_UL, 41);	// Texture unit 13
	}

	// Set the actual screen size
	GLint screenWidth_UnitLoc = glGetUniformLocation(shaderProgID, "screenWidth");
	GLint screenHeight_UnitLoc = glGetUniformLocation(shaderProgID, "screenHeight");

	glUniform1f(screenWidth_UnitLoc, FBO_map[pCurrentObject->friendlyName]->width);
	glUniform1f(screenHeight_UnitLoc, FBO_map[pCurrentObject->friendlyName]->height);

	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	glUniform1i(passNumber_UniLoc, 0);

	if (pCurrentObject->baseEffectUse == true)
	{
		GLint bUseBaseEffect_UL = glGetUniformLocation(shaderProgID, "bUseBaseEffect");
		glUniform1f(bUseBaseEffect_UL, (float)GL_FALSE);
	}

	if (pCurrentObject->overlayEffectTexture != "")
	{
		GLint bUseOverlayEffect_UL = glGetUniformLocation(shaderProgID, "bUseOverlayEffect");
		glUniform1f(bUseOverlayEffect_UL, (float)GL_FALSE);

		// Set up the texture for effect
		GLuint texSamp_ID = mpTexture[pCurrentObject->overlayEffectTexture];
		glActiveTexture(GL_TEXTURE0 + 41);				// Texture Unit 41
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0
		GLint texture_UL = glGetUniformLocation(shaderProgID, "effectTexture");
		glUniform1i(texture_UL, 41);	// Texture unit 13
	}

	return;
}

void cRenderer::RenderReflectiveObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	/*********************************************************************************/

	//SetUpSkyBoxTextureBindingsForObject(pCurrentObject, shaderProgID);

	GLint bIsReflective_UL = glGetUniformLocation(shaderProgID, "bIsReflective");
	glUniform1f(bIsReflective_UL, (float)GL_TRUE);

	GLuint skyBoxTextureID = FBO_map[pCurrentObject->friendlyName]->colourTexture_0_ID;
	glActiveTexture(GL_TEXTURE26);				// Texture Unit 26
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);	// Texture now assoc with texture unit 0

	// Tie the texture units to the samplers in the shader
	GLint skyBoxSampler1_UL = glGetUniformLocation(shaderProgID, "skyBox1");
	glUniform1i(skyBoxSampler1_UL, 26);	// Texture unit 26

	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	glUniform1f(bIsReflective_UL, (float)GL_FALSE);

	return;
}

void cRenderer::RenderAnimaObject(cAnimationComponent* pCurrentObject, sModelDrawInfo* drawInfo, GLint shaderProgID, cVAOManager* pVAOManager, GLint numBonesUsed, glm::f32* pVecFinalTransformtion){
	if (pCurrentObject->isVisible == false)
		return;

	//TODO: reorganize later

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// It's not SkyBox
	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	//SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	//Assume no discard texture
	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	//It's not Instanced
	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_FALSE);

	//Texture offset
	GLint useMovingTexture = glGetUniformLocation(shaderProgID, "useMovingTexture");
	glUniform1f(useMovingTexture, (float)GL_FALSE);

	//HeightMap offset
	GLint useHeightMap = glGetUniformLocation(shaderProgID, "useHeightMap");
	glUniform1f(useHeightMap, (float)GL_FALSE);


	/*********************************************************************************/

	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint isSkinnedMesh_UniLoc = glGetUniformLocation(shaderProgID, "isSkinnedMesh");
	glUniform1f(isSkinnedMesh_UniLoc, (float)GL_TRUE);

	GLint matBonesArray_UniLoc = glGetUniformLocation(shaderProgID, "matBonesArray");
	glUniformMatrix4fv(matBonesArray_UniLoc, numBonesUsed,
		GL_FALSE,
		pVecFinalTransformtion);


	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glm::mat4 transform(0.f);
	((cComplexObject*)pCurrentObject->GetParent())->GetPhysicsComponents()[0]->GetTransform(transform);
	transform *= pCurrentObject->GetScaleMatrix();
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(transform));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(transform));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, 0.f, 0.f, 1.f);
	//glUniform3f(newColour_location, pCurrentObject->pScene->mMeshes[0]->mColors[0]->r, pCurrentObject->pScene->mMeshes[0]->mColors[0]->g, pCurrentObject->pScene->mMeshes[0]->mColors[0]->b);
	//glUniform4f(diffuseColour_UL, pCurrentObject->pScene->mMeshes[0]->mColors[0]->r, pCurrentObject->pScene->mMeshes[0]->mColors[0]->g, pCurrentObject->pScene->mMeshes[0]->mColors[0]->b, pCurrentObject->pScene->mMeshes[0]->mColors[0]->a);
	//glUniform4f(specularColour_UL, pCurrentObject->pScene->mMeshes[0]->mColors[0]->r, pCurrentObject->pScene->mMeshes[0]->mColors[0]->g, pCurrentObject->pScene->mMeshes[0]->mColors[0]->b, pCurrentObject->pScene->mMeshes[0]->mColors[0]->a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");


	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform4f(debugColour_UL, 1.f, 0.f, 0.f, 1.f);
	glUniform1f(bDoNotLight_UL, (float)GL_TRUE);

	//Assume Solid
	//glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// DEPTH Test OFF/ON
	glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	glEnable(GL_DEPTH);


	/*********************************************************************************/
	glBindVertexArray(drawInfo->VAO_ID);
	glDrawElements(GL_TRIANGLES, drawInfo->numberOfIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	return;
}

void cRenderer::Render3DObjectMovingTexture(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, glm::vec2 offset) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}

	//Texture offset
	GLint useMovingTexture = glGetUniformLocation(shaderProgID, "useMovingTexture");
	glUniform1f(useMovingTexture, (float)GL_TRUE);
	GLint theOffset = glGetUniformLocation(shaderProgID, "textOffset");
	glUniform2f(theOffset, offset.x, offset.y);


	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	useMovingTexture = glGetUniformLocation(shaderProgID, "useMovingTexture");
	glUniform1f(useMovingTexture, (float)GL_FALSE);

	return;
}


void cRenderer::Render3DObjectHeightMap(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, glm::vec2 offset) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}

	//Texture offset
	GLint useMovingTexture = glGetUniformLocation(shaderProgID, "useMovingTexture");
	glUniform1f(useMovingTexture, (float)GL_TRUE);
	GLint theOffset = glGetUniformLocation(shaderProgID, "textOffset");
	glUniform2f(theOffset, offset.x, offset.y);


	//HeightMap offset
	GLint useHeightMap = glGetUniformLocation(shaderProgID, "useHeightMap");
	glUniform1f(useHeightMap, (float)GL_TRUE);

	GLuint heightMapID = mpTexture["WaterCausticHeightMap"];
	const int TEXTURE_UNIT_40 = 40;
	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_40);				// Texture Unit 40
	glBindTexture(GL_TEXTURE_2D, heightMapID);	// Texture now assoc with texture unit 0

	// Tie the texture units to the samplers in the shader
	GLint heightMap_UL = glGetUniformLocation(shaderProgID, "heightMap");
	glUniform1i(heightMap_UL, TEXTURE_UNIT_40);	// Texture unit 18

	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);
		
		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}
	return;

	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	useMovingTexture = glGetUniformLocation(shaderProgID, "useMovingTexture");
	glUniform1f(useMovingTexture, (float)GL_FALSE);

	useHeightMap = glGetUniformLocation(shaderProgID, "useHeightMap");
	glUniform1f(useHeightMap, (float)GL_FALSE);
}


void cRenderer::RenderDebugObject() {
	//TODO: Add code for the rendering of debug objects...
	return;
}


void cRenderer::RenderTextObject() {
	//TODO: Move the TextRender code here and call this to render the console window text...
	return;
}



void cRenderer::RenderSkyBoxObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// It's SkyBox
	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_TRUE);

	// Draw the back facing triangles. 
	// Because we are inside the object, so it will force a draw on the "back" of the sphere 
	//glCullFace(GL_FRONT_AND_BACK);
	glDisable(GL_CULL_FACE);	// Draw everything

	SetUpSkyBoxTextureBindingsForObject(pCurrentObject, shaderProgID);


	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON (Should always be off)
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON (Should always be off)
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		// Used to be glDrawElements
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,	// How many indices to draw
			GL_UNSIGNED_INT,
			0,							// Start index					
			0);							// Offset in the vertex buffer

		glBindVertexArray(0);
	}

	bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

	return;
}


void cRenderer::RenderImposterObject() {
	//TODO: Move imposter rendering here for any imposters that would be a onetime render...
	//NOTE: This would not happen very often and only in very rare cases...
	return;
}



void cRenderer::RenderInstanced3DObject() {
	//TODO: Move the rendering of instanced objects here...
	//NOTE: These are objects that are not particles or imposters; thus in the case we actually want to instance render full meshes of objects (like a massive amount of
	//		the same weapon/armor being drawn to the screen)
	return;
}



void cRenderer::RenderInstancedParticleObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, cParticleEmitter* pParticleEmitter) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}

	//It's Instanced
	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_TRUE);


	/*********************************************************************************/


	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON (If instanced should always be off)
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON (If instanced should always be off)
	(pCurrentObject->disableDepthBufferWrite) ? glDepthMask(GL_FALSE) : glDepthMask(GL_TRUE);

	// COLOR BUFFER Writting OFF/ON
	(pCurrentObject->disableColorBufferWrite) ? glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) : glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	/*********************************************************************************/


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		// Used to be glDrawElementsInstanced
		// glDrawElementsBaseVertex is needed if we'll be sending models to GPU in one buffer
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
			drawInfo.numberOfIndices,				// How many indices to draw
			GL_UNSIGNED_INT,
			0,										// Start index
			pParticleEmitter->getMaxParticles(),	// Number of instances
			0);										// Offset in the vertex buffer

		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);

		glBindVertexArray(0);
	}

	GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
	glUniform1f(bDiscard_UL, (float)GL_FALSE);

	bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_FALSE);

	return;
}



void cRenderer::RenderInstancedImposterObject() {
	//TODO: Move the rendering of imposter objects that are not particles here...
	//NOTE: This is likely to come up where an imposter will not be a particle but some texture effect on a 2d / 3d flat plane object that makes the
	//		player believe they are view some super cool effect when in reality they are not...
	//		i.e. light coronas.
	//		This effect on lightning or even the glow around lights would likely be a texture but here is a link for the idea of a light corona.
	//		https://en.wikipedia.org/wiki/Corona_discharge
	//		and son of a crap we now have another yes ANOTHER shader type as noted below
	//		https://en.wikipedia.org/wiki/Bloom_(shader_effect)
	//		this would also lead to both the tone mapping and colour mapping which may also lead to more textures being blended togeather!!!
	return;
}


void cRenderer::SetUpTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID) {
	if (pCurrentObject->v_textureNames.size() == 0)
	{
		GLuint ID = mpTexture["Default"];
		glActiveTexture(GL_TEXTURE0);		// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 0);	// Texture unit 0

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, 1.0f, 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 1)
	{
		GLuint ID = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE0);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 0);	// Texture unit 0

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 2)
	{
		GLuint ID0 = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE0);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID0);	// Texture now assoc with texture unit 0

		GLuint ID1 = mpTexture[pCurrentObject->v_textureNames[1]];
		glActiveTexture(GL_TEXTURE1);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID1);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 1);	// Texture unit 1

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], pCurrentObject->v_texureBlendRatio[1]);
	}

	return;
}

void cRenderer::SetUpSkyBoxTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID) {
	if (pCurrentObject->v_textureNames.size() == 0)
	{
		GLuint skyBoxTextureID = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE26);				// Texture Unit 26
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);	// Texture now assoc with texture unit 0


		// Tie the texture units to the samplers in the shader
		GLint skyBoxSampler1_UL = glGetUniformLocation(shaderProgID, "skyBox1");
		glUniform1i(skyBoxSampler1_UL, 26);	// Texture unit 26

		GLint skyBoxSampler2_UL = glGetUniformLocation(shaderProgID, "skyBox2");
		glUniform1i(skyBoxSampler2_UL, 26);	// Texture unit 26

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, 1.0f, 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 1)
	{
		GLuint skyBoxTextureID = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE26);				// Texture Unit 26
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);	// Texture now assoc with texture unit 0


		// Tie the texture units to the samplers in the shader
		GLint skyBoxSampler1_UL = glGetUniformLocation(shaderProgID, "skyBox1");
		glUniform1i(skyBoxSampler1_UL, 26);	// Texture unit 26

		GLint skyBoxSampler2_UL = glGetUniformLocation(shaderProgID, "skyBox2");
		glUniform1i(skyBoxSampler2_UL, 26);	// Texture unit 26

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 2)
	{
		GLuint skyBoxTextureID1 = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE26);				// Texture Unit 26
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID1);	// Texture now assoc with texture unit 0

		GLuint skyBoxTextureID2 = mpTexture[pCurrentObject->v_textureNames[1]];
		glActiveTexture(GL_TEXTURE27);				// Texture Unit 26
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID2);	// Texture now assoc with texture unit 0


		// Tie the texture units to the samplers in the shader
		GLint skyBoxSampler1_UL = glGetUniformLocation(shaderProgID, "skyBox1");
		glUniform1i(skyBoxSampler1_UL, 26);	// Texture unit 26

		GLint skyBoxSampler2_UL = glGetUniformLocation(shaderProgID, "skyBox2");
		glUniform1i(skyBoxSampler2_UL, 27);	// Texture unit 27

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], pCurrentObject->v_texureBlendRatio[1]);
	}

	return;
}